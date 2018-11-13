// Client.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
using json = nlohmann::json;
using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

int main()
{
	HttpServer server;
	server.config.port = 8080;

	server.resource["^/TEST$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		try 
		{
			response->write("Hello Client!!\n");
		}
		catch (const exception &e)
		{
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	thread server_thread([&server]() {server.start(); });

	HttpClient client("localhost:8080");
	try
	{
		auto req = client.request("POST", "/TEST");
		cout << req->content.string() << endl;
	}
	catch (const SimpleWeb::system_error &e)
	{
		cerr << "Client request error: " << e.what() << endl;
	}

	server_thread.join();
	return 0;
}

