#include "pch.h"
#include "hash.hpp"
#include "Block.hpp"
#include "common.hpp"
#include "BlockChain.hpp"
#include "json.hh"
#include "userinfolist.hpp"
#include "AES.hpp"
#pragma warning(disable:4996)

using json = nlohmann::json;
using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

MasterContainerClass MasterContainer;
UserInfoListClass UserInfoList;
map<string, string> NodeInfoList;
string MyNodeName = "Node2";

int main()
{
	NodeInfoList["Node1"] = "localhost:1111";
	NodeInfoList["Node2"] = "localhost:2222";
	NodeInfoList["Node3"] = "localhost:3333";
	UserInfoList.AddUser("Client1");
	MasterContainer.SetNodeInfoList(&NodeInfoList, MyNodeName);

	HttpServer server;
	server.config.port = 2222;

	server.resource["^/newchain$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {

		//채굴중지.
		MasterContainer.CancelMining();
		cout << "POST /newchain --- Node in Network sent new chain\n";
		try {
			json content = json::parse(request->content);
			if (content["length"].get<int>() > MasterContainer.m_BlockChain.getNumOfBlocks()) {
				MasterContainer.m_BlockChain.replaceChain(content);
				cout << "----Replaced current chain with new one" << endl;

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

		//cout << AESBase64File << endl;
		cout << RSAKey << endl;
		cout << DataName << endl;

		//./AESUserFiles 디렉터리 생성
		experimental::filesystem::path dir(".\\BackUpUserFiles");
		if (!(experimental::filesystem::exists(dir))) {
			experimental::filesystem::create_directory(dir);
		}

		//백업 파일 저장.
		std::ofstream out(string(".\\BackUpUserFiles\\") + RSAKey + DataName + string("back"), ios::binary);
		out << AESBase64File;
		out.close();

		try
		{
			cout << "Start to store Back up file\n" << endl;
			response->write("Success to store back up file\n");
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

		//cout << FileHash << endl;
		//cout << RSAKey << endl;
		//cout << DataName << endl;
		//cout << OriginFileAddress << endl;
		//cout << BackUpFileAddress << endl;
		//cout << "Start to store file hash\n" << endl;

		MasterContainer.InsertFileHash(FileHash);
		pair<int, int> ret = MasterContainer.GetCurBlockAndHashIndex();
		cout << ret.first << " " << ret.second << endl;

		//UserInfoList 갱신
		User& u = UserInfoList.FindUser(RSAKey);
		u.AddUserData(DataName, UserData(ret.first, ret.second, OriginFileAddress, BackUpFileAddress));

		try
		{
			response->write("Success to store file hash\n");
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

		//Base64형태로 인코딩된 string을 Base64로 디코딩함, 결과물은 AES로 암호화된 형태
		string DecodeBase64string = DecodeBase64String(AESBase64File);

		// AES로 암호화된 string을 sha256으로 해쉬,
		string FileHash = sha256(DecodeBase64string);

		//./AESUserFiles 디렉터리 생성
		experimental::filesystem::path dir(".\\AESUserFiles");
		if (!(experimental::filesystem::exists(dir))) {
			experimental::filesystem::create_directory(dir);
		}

		//원본 파일 저장.
		std::ofstream out(string(".\\AESUserFiles\\") + RSAKey + DataName, ios::binary);
		out << AESBase64File;
		out.close();

		//블럭체인 저장.		
		MasterContainer.InsertFileHash(FileHash);
		pair<int, int> ret = MasterContainer.GetCurBlockAndHashIndex();
		cout << ret.first << " " << ret.second << endl;

		//전파
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
					cout << "POST File Hash\n" << endl;
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
			cout << "POST Back up file\n" << endl;
			auto req1 = client1.request("POST", "/POSTBackUpFile", j1.dump());
			cout << req1->content.string() << endl;

			//나 자신의 UserInfoList 갱신
			User& ThisUser = UserInfoList.FindUser(RSAKey);
			ThisUser.AddUserData(DataName, UserData(ret.first, ret.second,
				OriginFileAddress, BackUpFileAddress));

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
			j2["DataPath"] = string(".\\AESUserFiles\\") + RSAKey + DataName;

			HttpClient client1(d.FileAddress);
			auto req2 = client1.request("POST", "/RequestData", j2.dump());
			//cout << req2->content.string() << endl;
			string data = req2->content.string();

			//데이터의 해시 계산하기
			string DecodeBase64string = DecodeBase64String(data);
			string hash0 = sha256(DecodeBase64string);

			while (check != 0) {}

			//계산 해시와 요청한 해시가 같은지 확인하기
			int hashcheck = 1;
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
				j2["DataPath"] = string(".\\BackUpUserFiles\\") + RSAKey + DataName + string("back");

				HttpClient client1(d.BackUpFileAddress);
				auto req2 = client1.request("POST", "/RequestData", j2.dump());
				string data = req2->content.string();

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

			response->write(data);
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	thread server_thread([&server]() {server.start(); });

	server_thread.join();
	return 0;
}


