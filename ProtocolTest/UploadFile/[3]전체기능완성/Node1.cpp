
#include "pch.h"

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



int main()
{
	NodeInfoList["Node1"] = "localhost:1111";
	NodeInfoList["Node2"] = "localhost:2222";
	NodeInfoList["Node3"] = "localhost:3333";
	UserInfoList.AddUser("Client1");
	MasterContainer.SetNodeInfoList(&NodeInfoList, MyNodeName);


	HttpServer server;
	server.config.port = 1111;


	server.resource["^/newchain$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {

		MasterContainer.m_MiningCancelFlag = true;
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

		string AESFile = DecodeBase64String(AESBase64File);

		//원본 파일 저장.
		std::ofstream out(string(".\\BackUpUserFiles\\") + RSAKey + DataName + string("back"), ios::binary);
		out << AESFile;
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

		cout << FileHash << endl;
		cout << RSAKey << endl;
		cout << DataName << endl;
		cout << OriginFileAddress << endl;
		cout << BackUpFileAddress << endl;
		cout << "Start to store file hash\n" << endl;

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
			if (u.UserName != RSAKey )
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
			std::ofstream out(string(".\\AESUserFiles\\") + RSAKey + DataName , ios::binary);
			out << DecodeBase64string;
			out.close();

			//블럭체인 저장.		
		    MasterContainer.InsertFileHash(FileHash);
			pair<int, int> ret = MasterContainer.GetCurBlockAndHashIndex();
			cout << ret.first << " " << ret.second << endl;

								
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
				auto req1 = client1.request("POST", "/POSTBackUpFile",j1.dump());
				cout << req1->content.string() << endl;
							
				//나 자신의 UserInfoList 갱신
				User& ThisUser = UserInfoList.FindUser(RSAKey);
				ThisUser.AddUserData(DataName, UserData(ret.first, ret.second,
					OriginFileAddress, BackUpFileAddress));

			});
			thread.detach();
			

		try{
			cout << "sending Success message" << endl;
			response->write("UploadFile Success\n");
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


