
#include "pch.h"

using json = nlohmann::json;
using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;


int main()
{

	HttpServer server;
	server.config.port = 3333;


	server.resource["^/POSTBackUpFile$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		json content1 = json::parse(request->content);
		string AESBase64File = content1["AESBase64File"].get<string>();
		string RSAKey = content1["RSAKey"].get<string>();
		string DataName = content1["DataName"].get<string>();

		cout << AESBase64File << endl;
		cout << RSAKey << endl;
		cout << RSAKey << endl;

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

		try
		{
			cout << "Start to store file hash\n" << endl;
			response->write("Success to store file hash\n");
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	server.resource["^/UploadFile$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try
		{

			json content1 = json::parse(request->content);
			string AESBase64File = content1["AESBase64File"].get<string>();
			string RSAKey = content1["RSAKey"].get<string>();
			string DataName = content1["DataName"].get<string>();

			cout << AESBase64File << endl;
			cout << RSAKey << endl;
			cout << RSAKey << endl;

			thread thread([=]
			{
				json j1;
				j1["DataName"] = DataName.c_str();
				j1["RSAKey"] = RSAKey.c_str();
				j1["AESBase64File"] = AESBase64File.c_str();

				HttpClient client1("localhost:1111");
				cout << "POST Back up file\n" << endl;
				auto req1 = client1.request("POST", "/POSTBackUpFile", j1.dump());
				cout << req1->content.string() << endl;


				json j2;
				j2["FileHash"] = "Hash Value test";
				j2["RSAKey"] = RSAKey.c_str();
				j2["DataName"] = DataName.c_str();
				j2["OriginFileAddress"] = "localhost:1111";
				j2["BackUpFileAddress"] = "localhost:2222";

				HttpClient client2("localhost:1111");
				cout << "POST File Hash\n" << endl;
				auto req2 = client2.request("POST", "/POSTFileHash", j2.dump());
				cout << req2->content.string() << endl;


				json j3;
				j3["FileHash"] = "Hash Value test";
				j3["RSAKey"] = RSAKey.c_str();
				j3["DataName"] = DataName.c_str();
				j3["OriginFileAddress"] = "localhost:1111";
				j3["BackUpFileAddress"] = "localhost:2222";

				HttpClient client3("localhost:2222");
				cout << "POST File Hash\n" << endl;
				auto req3 = client3.request("POST", "/POSTFileHash", j3.dump());
				cout << req3->content.string() << endl;
			});
			thread.detach();


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

			json content1 = json::parse(request->content);
			string AESBase64File = content1["AESBase64File"].get<string>();
			string RSAKey = content1["RSAKey"].get<string>();
			string DataName = content1["DataName"].get<string>();

			cout << AESBase64File << endl;
			cout << RSAKey << endl;
			cout << RSAKey << endl;

			/*
			thread thread([=]
			{
				json j1;
				j1["DataName"] = DataName.c_str();
				j1["RSAKey"] = RSAKey.c_str();
				j1["AESBase64File"] = AESBase64File.c_str();

				HttpClient client1("localhost:2222");
				cout << "POST Back up file\n" << endl;
				auto req1 = client1.request("POST", "/POSTBackUpFile", j1.dump());
				cout << req1->content.string() << endl;


				json j2;
				j2["FileHash"] = "Hash Value test";
				j2["RSAKey"] = RSAKey.c_str();
				j2["DataName"] = DataName.c_str();
				j2["OriginFileAddress"] = "localhost:1111";
				j2["BackUpFileAddress"] = "localhost:2222";

				HttpClient client2("localhost:2222");
				cout << "POST File Hash\n" << endl;
				auto req2 = client2.request("POST", "/POSTFileHash", j2.dump());
				cout << req2->content.string() << endl;


				json j3;
				j3["FileHash"] = "Hash Value test";
				j3["RSAKey"] = RSAKey.c_str();
				j3["DataName"] = DataName.c_str();
				j3["OriginFileAddress"] = "localhost:1111";
				j3["BackUpFileAddress"] = "localhost:2222";

				HttpClient client3("localhost:3333");
				cout << "POST File Hash\n" << endl;
				auto req3 = client3.request("POST", "/POSTFileHash", j3.dump());
				cout << req3->content.string() << endl;
			});
			thread.detach();
			*/

			cout << "request download" << endl;
			response->write("Download File Success\n");

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
			int check1, check2;
			check1 = check2 = 0;

			json content1 = json::parse(request->content);
			string AESBase64File = content1["AESBase64File"].get<string>();
			string RSAKey = content1["RSAKey"].get<string>();
			string DataName = content1["DataName"].get<string>();

			cout << AESBase64File << endl;
			cout << RSAKey << endl;
			cout << DataName << endl << endl;

			cout << "RequestHash" << endl;
			cout << "GetHash" << endl;
			cout << "returnhash" << endl;

			response->write("returnhash Success\n");

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


