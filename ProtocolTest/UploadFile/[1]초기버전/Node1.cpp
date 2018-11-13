
#include "pch.h"

using json = nlohmann::json;
using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;


int main()
{

	HttpServer server;
	server.config.port = 1111;


	server.resource["^/POSTBackUpFile$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
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
			
			thread thread1([]()
			{
				HttpClient client("localhost:2222");
				cout << "POST Back up file\n" << endl;
				auto req1 = client.request("POST", "/POSTBackUpFile");
				cout << req1->content.string() << endl;
			});
			thread1.detach();

			thread thread2([]()
			{
				HttpClient client("localhost:2222");
				cout << "POST Back up file\n" << endl;
				auto req1 = client.request("POST", "/POSTBackUpFile");
				cout << req1->content.string() << endl;
			});
			thread2.detach();

			thread thread3([]()
			{
				HttpClient client("localhost:3333");
				cout << "POST File Hash\n" << endl;
				auto req3 = client.request("POST", "/POSTFileHash");
				cout << req3->content.string() << endl;
			});
			thread3.detach();

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

