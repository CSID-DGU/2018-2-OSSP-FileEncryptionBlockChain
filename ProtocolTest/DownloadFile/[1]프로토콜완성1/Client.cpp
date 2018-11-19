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
		json j;
		j["AESBase64File"] = "Test file";
		j["RSAKey"] = "Test RSA Key";
		j["DataName"] = "Test Data Name";
		
		cout << "POST UploadFile\n" << endl;
		//auto req = client.request("POST", "/UploadFile",j.dump());
		auto req = client.request("POST", "/DownloadFile", j.dump());
		cout << req->content.string() << endl;
	}
	catch (const SimpleWeb::system_error &e)
	{
		cerr << "Client request error: " << e.what() << endl;
	}
	
	return 0;
}