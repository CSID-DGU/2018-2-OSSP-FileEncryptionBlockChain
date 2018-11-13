// Client.cpp : �� ���Ͽ��� 'main' �Լ��� ���Ե˴ϴ�. �ű⼭ ���α׷� ������ ���۵ǰ� ����˴ϴ�.
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