#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include "requests.hpp"
#include "json.hh"
#include "client_http.hpp"
#include "server_http.hpp"

#include "hash.hpp"
#include "Block.hpp"
#include "common.hpp"
#include "BlockChain.hpp"
#include "json.hh"
#include "userinfolist.hpp"
#include "AES.hpp"

using json = nlohmann::json;
using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

MasterContainerClass MasterContainer;
UserInfoListClass UserInfoList;
map<string, string> NodeInfoList;
string MyNodeName = "Node1";
string MyNodeAddress = "localhost:1111";




int main()
{
	//NodeInfoList["Node1"] = "localhost:1111";
	//NodeInfoList["Node2"] = "localhost:2222";
	//NodeInfoList["Node3"] = "localhost:3333";
	NodeInfoList[MyNodeName] = MyNodeAddress;
	//UserInfoList.AddUser("Client1");
	MasterContainer.SetNodeInfoList(&NodeInfoList, MyNodeName);
	//MasterContainer.SetOwnerUser("Client1");
	MasterContainer.SetUserInfoList(&UserInfoList);


	HttpServer server;
	server.config.port = 1111;

	server.resource["^/GETBlockFiles"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		json content = json::parse(request->content);
		json Blockfiles = MasterContainer.m_BlockChain.HandleBlockFilesTransfer(content);
		try {
			response->write(Blockfiles.dump());
		}
		catch (const exception &e) {
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/newchain$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		json content = json::parse(request->content);
		json BlockChain = json::parse(content["BlockChain"].get<string>());

		cout << "POST /newchain --- Node in Network sent new chain\n" << endl;
		try {

			if (content["BlockNum"].get<int>() > MasterContainer.m_BlockChain.getNumOfBlocks()) {
				//채굴중지.
				MasterContainer.CancelMining();

				if (content["ReplaceType"].get<int>() == 1)
				{
					MasterContainer.m_BlockChain.replaceChain(BlockChain);
				}
				else if (content["ReplaceType"].get<int>() == 2)
				{
					//Increase Block Num
					MasterContainer.m_BlockChain.m_BlockNum = content["BlockNum"].get<int>();
					MasterContainer.m_BlockChain.replaceChain2(BlockChain);
				}

				//Increase Block Num
				MasterContainer.m_BlockChain.m_BlockNum = content["BlockNum"].get<int>();
				cout << "----Replaced current chain with new one " << MasterContainer.m_BlockChain.m_BlockNum << endl;

				response->write("Replaced Chain\n");
			}
			else {
				cout << "----Chain was not replaced: sent chain had same size" << endl;
				response->write("Same Chain Size -- invalid");
			}
		}
		catch (const exception &e) {
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/POSTBackUpFile$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		json content1 = json::parse(request->content);
		string AESBase64File = content1["AESBase64File"].get<string>();
		string RSAKey = content1["RSAKey"].get<string>();
		string DataName = content1["DataName"].get<string>();

		cout << "Start to store Back up file" << endl;
		cout << "* Client Name :" << RSAKey << endl;
		cout << "* Data Name :" << DataName << endl;

		//./AESUserFiles 디렉터리 생성
		experimental::filesystem::path dir("BackUpUserFiles");
		if (!(experimental::filesystem::exists(dir))) {
			experimental::filesystem::create_directory(dir);
		}

		//string AESFile = DecodeBase64String(AESBase64File);

		//원본 파일 저장.
		std::ofstream out(string("BackUpUserFiles\\") + RSAKey + DataName + string("back"), ios::binary);
		out << AESBase64File;
		out.close();

		try
		{
			response->write("Success to store back up file");
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/POSTFileHash$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		json content1 = json::parse(request->content);
		string FileHash = content1["FileHash"].get<string>();
		string RSAKey = content1["RSAKey"].get<string>();
		string DataName = content1["DataName"].get<string>();
		string OriginFileAddress = content1["OriginFileAddress"].get<string>();
		string BackUpFileAddress = content1["BackUpFileAddress"].get<string>();

		cout << "File Hash output" << endl;
		cout << "* Hash Value :" << FileHash << endl;
		cout << "* Client Name :" << RSAKey << endl;
		cout << "* Data Name :" << DataName << endl;
		cout << "* OrgAddress :" << OriginFileAddress << endl;
		cout << "* BackUpAddress :" << BackUpFileAddress << endl;
		cout << "Start to store file hash" << endl;

		MasterContainer.InsertFileHash(FileHash);
		pair<int, int> ret = MasterContainer.GetCurBlockAndHashIndex();
		cout << "Stored at : " << ret.first << " " << ret.second << endl;

		//UserInfoList 갱신
		User& u = UserInfoList.FindUser(RSAKey);
		u.AddUserData(DataName, UserData(ret.first, ret.second, OriginFileAddress, BackUpFileAddress));


		try
		{
			response->write("Success to store file hash");
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/UploadFile$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		json content1 = json::parse(request->content);
		string AESBase64File = content1["AESBase64File"].get<string>();
		string RSAKey = content1["RSAKey"].get<string>();
		string DataName = content1["DataName"].get<string>();

		//UserInfoList에 있는지 확인
		User& u = UserInfoList.FindUser(RSAKey);
		if (u.UserName != RSAKey)
		{
			cout << " Invalid User UploadFile Reques !!" << endl;
			response->write("UploadFile Failed\n");
			return;
		}

		//이미 이 데이터 네임이 존재하는지 확인.
		if (u.IsExistData(DataName) == true)
		{
			cout << "This data already exists. !!" << endl;
			response->write("UploadFile Failed\n");
			return;
		}
		//파입 업로드 가능 횟수 확인 밑 처리
		if (u.IsExhaustedCount())
		{
			cout << "This User no more upload file. !!" << endl;
			response->write("UploadFile Failed\n");
			return;
		}
		// 파일업로드 가능 횟수 1 감소
		u.DecrementUsageCount();


		//Base64형태로 인코딩된 string을 Base64로 디코딩함, 결과물은 AES로 암호화된 형태
		string DecodeBase64string = DecodeBase64String(AESBase64File);

		// AES로 암호화된 string을 sha256으로 해쉬,
		string FileHash = sha256(DecodeBase64string);

		//./AESUserFiles 디렉터리 생성
		experimental::filesystem::path dir("AESUserFiles");
		if (!(experimental::filesystem::exists(dir))) {
			experimental::filesystem::create_directory(dir);
		}

		//원본 파일 저장.
		std::ofstream out(string("AESUserFiles\\") + RSAKey + DataName, ios::binary);
		out << AESBase64File;
		out.close();

		//블럭체인 저장.		
		MasterContainer.InsertFileHash(FileHash);
		pair<int, int> ret = MasterContainer.GetCurBlockAndHashIndex();
		cout << "Stored at : " << ret.first << " " << ret.second << endl;


		thread thread([=]
		{
			auto it1 = NodeInfoList.find(MyNodeName);
			if (++it1 == NodeInfoList.end()) {
				it1 = NodeInfoList.begin();
			}
			string BackUpFileAddress = it1->second;
			string OriginFileAddress = NodeInfoList[MyNodeName];


			//파일해쉬 전파
			json j2;
			j2["FileHash"] = FileHash;
			j2["RSAKey"] = RSAKey.c_str();
			j2["DataName"] = DataName.c_str();
			j2["OriginFileAddress"] = OriginFileAddress;
			j2["BackUpFileAddress"] = BackUpFileAddress;

			for (auto it2 = NodeInfoList.begin(); it2 != NodeInfoList.end(); it2++)
			{
				if (it2->first != MyNodeName)
				{
					string Address = it2->second;
					HttpClient client2(Address);
					cout << "POST File Hash" << endl;
					auto req2 = client2.request("POST", "/POSTFileHash", j2.dump());
					cout << req2->content.string() << endl;
				}
			}


			//백업 파일 전송.
			json j1;
			j1["DataName"] = DataName.c_str();
			j1["RSAKey"] = RSAKey.c_str();
			j1["AESBase64File"] = AESBase64File.c_str();

			HttpClient client1(BackUpFileAddress);
			cout << "POST Back up file" << endl;
			auto req1 = client1.request("POST", "/POSTBackUpFile", j1.dump());
			cout << req1->content.string() << endl;

			//나 자신의 UserInfoList 갱신
			User& ThisUser = UserInfoList.FindUser(RSAKey);
			ThisUser.AddUserData(DataName, UserData(ret.first, ret.second,
				OriginFileAddress, BackUpFileAddress));

			cout << "File Hash output" << endl;
			cout << "* Hash Value :" << FileHash << endl;
			cout << "* Client Name :" << RSAKey << endl;
			cout << "* Data Name :" << DataName << endl;
			cout << "* OrgAddress :" << OriginFileAddress << endl;
			cout << "* BackUpAddress :" << BackUpFileAddress << endl;
			cout << "Start to store file hash" << endl;

		});
		thread.detach();


		try {
			cout << "sending Success message" << endl;
			response->write("UploadFile Success\n");
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};


	server.resource["^/DownloadFile$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{
			int check;
			int hashsize = 0;
			string hash[100];

			json content1 = json::parse(request->content);
			string RSAKey = content1["RSAKey"].get<string>();
			string DataName = content1["DataName"].get<string>();

			//해당 유저가 있는지 확인
			User& u = UserInfoList.FindUser(RSAKey);
			if (u.UserName != RSAKey)
			{
				cout << " Invalid User UploadFile Reques !!" << endl;
				response->write("DownloadFile Failed\n");
				return;
			}

			//해당 데이터가 존재하는지 확인.
			if (u.IsExistData(DataName) != true)
			{
				cout << "This data not exists. !!" << endl;
				response->write("DownloadFile Failed\n");
				return;
			}

			//해당 데이터의 정보 확인
			UserData& d = u.FindData(DataName);

			//데이터의 해시 요청하기
			thread thread([&]
			{
				check = 1;

				auto it1 = NodeInfoList.find(MyNodeName);
				if (++it1 == NodeInfoList.end()) {
					it1 = NodeInfoList.begin();
				}

				json j1;
				j1["Blockindex"] = d.BlockIndex;
				j1["Hashindex"] = d.HashIndex;

				for (auto it2 = NodeInfoList.begin(); it2 != NodeInfoList.end(); it2++)
				{
					if (it2->first != MyNodeName)
					{
						string Address = it2->second;
						HttpClient client2(Address);
						auto req1 = client2.request("POST", "/RequestHash", j1.dump());
						hash[hashsize] = req1->content.string();
						hashsize++;
					}
				}
				//자신의 블럭 내의 해시값 확인
				hash[hashsize] = MasterContainer.GetFileHash(d.BlockIndex, d.HashIndex);
				hashsize++;

				check = 0;
			});
			thread.detach();

			//데이터 요청하기
			json j2;
			j2["DataPath"] = string("AESUserFiles\\") + RSAKey + DataName;

			string data;
			//데이터가 외부에 있을경우 데이터요청
			if (d.FileAddress != NodeInfoList[MyNodeName]) {
				HttpClient client1(d.FileAddress);
				auto req2 = client1.request("POST", "/RequestData", j2.dump());
				data = req2->content.string();
			}
			//데이터가 로컬에 있을경우 데이터 요청
			else{
				string dataname = j2["DataPath"].get<string>();

				std::ifstream infile(dataname, std::ios::binary);
				infile.seekg(0, infile.end);
				size_t length = infile.tellg();
				infile.seekg(0, infile.beg);
				char *pBuffer = new char[length + 1];
				memset(pBuffer, 0, length + 1);
				infile.read(pBuffer, length);

				data = pBuffer;
				delete pBuffer;
			}

			//데이터의 해시 계산하기
			string DecodeBase64string = DecodeBase64String(data);
			string hash0 = sha256(DecodeBase64string);

			while (check != 0) {}

			//계산 해시와 요청한 해시가 같은지 확인하기
			int hashcheck = 0;
			int i = 0;
			while (i < hashsize)
			{
				string a = hash[i];
				if (hash0 != hash[i])	hashcheck++;
				i++;
			}

			//성공, 백업확인, 실패
			if (hashcheck == 0)
			{
				response->write(data);
			}
			else
			{
				//다른 경우 백업파일의 해시를 받아서 다시 계산후 비교
				json j2;
				j2["DataPath"] = string("BackUpUserFiles\\") + RSAKey + DataName + string("back");

				string data;
				//데이터가 외부에 있을경우
				if (d.BackUpFileAddress != NodeInfoList[MyNodeName]) {
					HttpClient client1(d.BackUpFileAddress);
					auto req2 = client1.request("POST", "/RequestData", j2.dump());
					data = req2->content.string();
				}
				//데이터가 로컬에 있을경우
				else {
					string dataname = j2["DataPath"].get<string>();

					std::ifstream infile(dataname, std::ios::binary);
					infile.seekg(0, infile.end);
					size_t length = infile.tellg();
					infile.seekg(0, infile.beg);
					char *pBuffer = new char[length + 1];
					memset(pBuffer, 0, length + 1);
					infile.read(pBuffer, length);

					data = pBuffer;
					delete pBuffer;


				}

				//데이터의 해시 계산하기
				string DecodeBase64string = DecodeBase64String(data);
				string hash0 = sha256(DecodeBase64string);

				//계산 해시와 요청한 해시가 같은지 확인하기
				int hashcheck = 0;
				int i = 0;
				while (i < hashsize)
				{
					string a = hash[i];
					if (hash0 != hash[i])	hashcheck++;
					i++;
				}

				if (hashcheck == 0)
				{
					response->write(data);
				}
				else
				{
					response->write("fail");
				}
			}

		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/RequestHash$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{
			json content1 = json::parse(request->content);
			int blocki = content1["Blockindex"].get<int>();
			int hashi = content1["Hashindex"].get<int>();

			string hash = MasterContainer.GetFileHash(blocki, hashi);
			response->write(hash);
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/RequestData$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{
			json content1 = json::parse(request->content);
			string dataname = content1["DataPath"].get<string>();

			std::ifstream infile(dataname, std::ios::binary);
			infile.seekg(0, infile.end);
			size_t length = infile.tellg();
			infile.seekg(0, infile.beg);
			char *pBuffer = new char[length + 1];
			memset(pBuffer, 0, length + 1);
			infile.read(pBuffer, length);

			string data = pBuffer;

			delete pBuffer;
			response->write(data);
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/RequestList$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{
			json content1 = json::parse(request->content);
			string RSAKey = content1["RSAKey"].get<string>();

			//해당 유저가 있는지 확인
			User& u = UserInfoList.FindUser(RSAKey);
			if (u.UserName != RSAKey)
			{
				cout << " Invalid User UploadFile Reques !!" << endl;
				response->write("DownloadFile Failed\n");
				return;
			}

			//해당 유저의 모든 데이터명 확인, 연결
			map<string, UserData>::iterator iter;
			map<string, UserData> v = u.UserDataMap;

			string data;
			for (iter = v.begin(); iter != v.end(); ++iter) {
				data = data + iter->first + '\n';
			}
			data += string("Usage Count : ") + to_string(u.UsageCount);

			//해당 유저의 데이터명 리스트 반환
			response->write(data);

		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

		server.resource["^/RequestUserInfoList$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		json content1 = json::parse(request->content);
		string RequestNodeName = content1["Name"].get<string>();

		response->write(UserInfoList.toJSON().dump());
	};



	server.resource["^/NewClientSignUp$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		int check2 = 0;

		try
		{

			json content1 = json::parse(request->content);
			string ClientName = content1["Name"].get<string>();

			string message;
			bool admit = false;
			char check1;

			cout << ClientName << endl;


			cout << "승인하시겠습니까?(y/n)" << endl;
			cin >> check1;

			if (check1 == 'y') {
				admit = true;
			}
			else {
				admit = false;
			}


			if (admit) {

				cout << "전파" << endl;

				/*전파*/
thread thread([&]
{
	//전파할 내용
	json j1;
	j1["Name"] = ClientName;

	//승인했는지 체크하는 변수
	string admitCheck;


	for (auto it = NodeInfoList.begin(); it != NodeInfoList.end(); it++)
	{
		if (it->first != MyNodeName)
		{
			string Address = it->second;
			HttpClient client2(Address);
			auto req2 = client2.request("POST", "/SendNewClientInfo", j1.dump());
			admitCheck = req2->content.string();
			cout << admitCheck << endl;

			if (admitCheck._Equal("fail")) {
				check2++;
			}
		}
	}
	if (check2 == 0) {
		check2 = -1;
	}
});
while (check2 == 0) {}
thread.detach();


if (check2 < 0) {

	cout << "New Client ID 전파" << endl;

	UserInfoList.AddUser(ClientName);
	int count = 0;

	//전파할 내용
	json j2;
	j2["Name"] = ClientName;

	for (auto it = NodeInfoList.begin(); it != NodeInfoList.end(); it++)
	{
		if (it->first != MyNodeName)
		{
			string Address = it->second;
			HttpClient client2(Address);
			auto req2 = client2.request("POST", "/RefreshUserInfoList", j2.dump());
			cout << req2->content.string() << endl;
		}
	}

	response->write("success");
}
else {
	response->write("fail");
}
			}
			else {
			response->write("fail");
			}
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/SendNewClientInfo$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{

			json content1 = json::parse(request->content);
			string ClientName = content1["Name"].get<string>();


			string message;
			bool admit = false;
			char check;

			cout << ClientName << endl;

			cout << "승인하시겠습니까?(y/n)" << endl;
			cin >> check;

			if (check == 'y') {
				admit = true;
			}
			else {
				admit = false;
			}

			if (admit) {
				message = "admit";
			}
			else {
				message = "fail";
			}
			/*
			thread thread([=]
			{
				json j1;
				j1["Admit"] = message;

				HttpClient client1("localhost:2222");
				cout << "Request NodeInfoList\n" << endl;
				auto req1 = client1.request("POST", "/RequestNodeInfoList", j1.dump());
				cout << req1->content.string() << endl;

			});
			thread.detach();
			*/
			response->write(message);

		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/RefreshUserInfoList$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{
			json content1 = json::parse(request->content);
			string ClientName = content1["Name"].get<string>();


			cout << ClientName << endl;

			UserInfoList.AddUser(ClientName);

			response->write("Add User Success\n");

		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};


	server.resource["^/NewNodeInfo$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		int check = 0;

		try
		{

			json content1 = json::parse(request->content);
			string NodeName = content1["Name"].get<string>();
			string NetworkAddress = content1["NetworkAddress"].get<string>();

			string message;
			bool admit = false;
			char check1;

			cout << NodeName << endl;
			cout << NetworkAddress << endl;

			cout << "승인하시겠습니까?(y/n)" << endl;
			cin >> check1;

			if (check1 == 'y') {
				admit = true;
			}
			else {
				admit = false;
			}


			if (admit) {

				cout << "전파" << endl;

				/*전파*/
				thread thread([&]
				{
					//전파할 내용
					json j1;
					j1["Name"] = NodeName;
					j1["NetworkAddress"] = NetworkAddress;

					//승인했는지 체크하는 변수
					string admitCheck;


					for (auto it = NodeInfoList.begin(); it != NodeInfoList.end(); it++)
					{
						if (it->first != MyNodeName)
						{
							string Address = it->second;
							HttpClient client2(Address);
							auto req2 = client2.request("POST", "/SendNodeInfo", j1.dump());
							admitCheck = req2->content.string();
							cout << admitCheck << endl;

							if (admitCheck._Equal("fail")) {
								check++;
							}
						}
					}
					if (check == 0) {
						check = -1;
					}
				});
				while (check == 0) {}
				thread.detach();


				if (check < 0) {

					cout << "NodeInfoList전파" << endl;

					NodeInfoList[NodeName] = NetworkAddress;
					int count = 0;

					//전파할 내용
					json j2;
					int i = 0;
					for (auto it = NodeInfoList.begin(); it != NodeInfoList.end(); it++) {
						j2[to_string(i)]["Name"] = it->first;
						j2[to_string(i)]["NetworkAddress"] = it->second;
						i++;
					}
					j2["size"] = to_string(NodeInfoList.size());

					for (auto it = NodeInfoList.begin(); it != NodeInfoList.end(); it++)
					{
						if (it->first != MyNodeName)
						{
							string Address = it->second;
							HttpClient client2(Address);
							auto req2 = client2.request("POST", "/RefreshNodeInfoList", j2.dump());
							cout << it->first << endl << endl;
							cout << req2->content.string() << endl;
						}
					}

					response->write("success");
				}
				else {
					response->write("fail");
				}
			}
			else {
				response->write("fail");
			}
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/SendNodeInfo$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{

			json content1 = json::parse(request->content);
			string NodeName = content1["Name"].get<string>();
			string NetworkAddress = content1["NetworkAddress"].get<string>();

			string message;
			bool admit = false;
			char check;

			cout << NodeName << endl;
			cout << NetworkAddress << endl;

			cout << "승인하시겠습니까?(y/n)" << endl;
			cin >> check;

			if (check == 'y') {
				admit = true;
			}
			else {
				admit = false;
			}

			if (admit) {
				message = "admit";
			}
			else {
				message = "fail";
			}
			/*
			thread thread([=]
			{
				json j1;
				j1["Admit"] = message;

				HttpClient client1("localhost:2222");
				cout << "Request NodeInfoList\n" << endl;
				auto req1 = client1.request("POST", "/RequestNodeInfoList", j1.dump());
				cout << req1->content.string() << endl;

			});
			thread.detach();
			*/
			response->write(message);

		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/RefreshNodeInfoList"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{
			json content1 = json::parse(request->content);
			string size = content1["size"].get<string>();
			int listSize = atoi(size.c_str());

			string Name;
			string NetworkAddress;

			cout << size << endl;

			for (int i = 0; i < listSize; i++) {
				Name = content1[to_string(i)]["Name"].get<string>();
				NetworkAddress = content1[to_string(i)]["NetworkAddress"].get<string>();
				NodeInfoList[Name] = NetworkAddress;
			}

			for (auto it = NodeInfoList.begin(); it != NodeInfoList.end(); it++) {
				cout << it->first << ", " << it->second << endl;
			}
			cout << endl;


			response->write("Add Node Success\n");

		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/RequestBlockChain"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		json content1 = json::parse(request->content);
		string RequestNodeAddress = content1["Address"].get<string>();

		thread thread1([=] {
			HttpClient client(RequestNodeAddress);
			json j;
			j["ReplaceType"] = 1;
			j["BlockNum"] = MasterContainer.m_BlockChain.getNumOfBlocks();
			j["BlockChain"] = MasterContainer.m_BlockChain.toJSON();

			try {
				auto req = client.request("POST", "/newchain", j.dump());
				cout << "BlockChainSend" << endl;
			}
			catch (const SimpleWeb::system_error &e) {
				cerr << "Client request error: " << e.what() << endl;
			}
		});

		thread1.detach();
	
		try {
			response->write("Send Memory Block Chain\n");
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}


	};


	thread server_thread([&server]() {server.start(); });

	//프롬프트
	while (1)
	{
		string UserInput;
		cout << "\nCommand> " ;
		cin >> UserInput;

		if (UserInput == "g1")
		{
			cout << MasterContainer.toJSON().dump(3);
		}
		else if (UserInput == "g2")//userinfolist 출력
		{
			cout << UserInfoList.toJSON().dump(3) << endl;

		}
		else if (UserInput == "g3")//nodeinfolist 출력
		{
			cout << "nodeinfolist" << endl;
			auto it1 = NodeInfoList.begin();
			for (it1 = NodeInfoList.begin(); it1 != NodeInfoList.end(); ++it1) {
				cout << it1->first << " : " << it1->second << endl;
			}

		}
		else if (UserInput == "g4")
		{
			cout << "[Node owner User] : " << MasterContainer.m_OwnerUser << endl;
			cout << "[Node Name] : " << MyNodeName << endl;
		}
		else if (UserInput == "s1")
		{
			string UserInput2;
			cout << "Input Owner Name >";
			cin >> UserInput2;
			MasterContainer.SetOwnerUser(UserInput2);
			cout << "Node owner Chainged to " + UserInput2 << endl;
		}
		else if (UserInput == "s2")
		{
			string UserInput2;
			cout << "Input Node Name >";
			cin >> UserInput2;
			MyNodeName = UserInput2;
			cout << "Node Name Chainged to " + UserInput2 << endl;
			string UserInput3;
			cout << "Input Node Address >";
			cin >> UserInput3;
			MyNodeAddress = UserInput3;
			cout << "Node Name Chainged to " + MyNodeAddress << endl;
			NodeInfoList[UserInput2] = UserInput3;
		}
		else if (UserInput == "s3")
		{
			string admit;
			string UserInput2;
			cout << "Input BlockChain Node Address >";
			cin >> UserInput2;

			json NodeInfo;
			NodeInfo["Name"] = MyNodeName;
			NodeInfo["NetworkAddress"] = MyNodeAddress;

			HttpClient client(UserInput2);
			cout << "send node name, address\n" << endl;
			auto req = client.request("POST", "/NewNodeInfo", NodeInfo.dump());
			admit = req->content.string();
			cout << admit << endl;

			
			if (admit == "success")

			{	//UserInfoList 전송요청
				HttpClient client2(UserInput2);
				json NodeInfo2;
				NodeInfo2["Name"] = MyNodeName;
				cout << "Request UserInfoList\n" << endl;
				auto req2 = client.request("POST", "/RequestUserInfoList", NodeInfo.dump());
				string UserInfoListString = req2->content.string();
				json UserInfoListJson = json::parse(UserInfoListString);
				UserInfoList.ReBuildUserInfoList(UserInfoListJson);				
				cout << UserInfoList.toJSON().dump(3) << endl;


				//블록체인 파일 전송 요청
				if (MasterContainer.m_BlockChain.ReplaceBlockFiles(&NodeInfoList, MyNodeName)){
					cout << "Success to Get block chain files" << endl;
				}
				else {
					cout << "Failed to Get block chain files" << endl;
				}

				//메모리 블록체인 교체 요청
				HttpClient client3(UserInput2);
				json RequestBlockChain;
				RequestBlockChain["Address"] = MyNodeAddress;
				auto req3 = client.request("POST", "/RequestBlockChain", RequestBlockChain.dump());
				string Result = req2->content.string();
				cout << Result << endl;		
			}
		}
		else
		{
			cout << "* Command is Not founded" << endl;
			cout << "* g1 : Print MasterContainer" << endl;
			cout << "* g2 : Print UserInfoList" << endl;
			cout << "* g3 : Print NodeInfoList" << endl;
			cout << "* g4 : Print Node owner and Node Name" << endl;
			cout << "* s1 : Set owner User" << endl;
			cout << "* s2 : Set Node Name" << endl;
			cout << "* s3 : Connect to BlockChain Network" << endl;
		}
	}

	server_thread.join();
	return 0;
}


