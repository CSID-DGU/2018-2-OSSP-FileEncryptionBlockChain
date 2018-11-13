// Client.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
using json = nlohmann::json;
using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

int main()
{

	HttpClient client("localhost:1111");
	try
	{	
		cout << "POST UploadFile\n" << endl;
		auto req = client.request("POST", "/UploadFile");
		cout << req->content.string() << endl;
	}
	catch (const SimpleWeb::system_error &e)
	{
		cerr << "Client request error: " << e.what() << endl;
	}
	
	return 0;
}