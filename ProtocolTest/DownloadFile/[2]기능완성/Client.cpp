
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

string MyName = "Client1";
map<string, string> NodeInfoList;


int main()
{	
	
	// /KEY 폴더에 AES키와 암호화를 위한 이니셜벡터 두가지 파일이 있는지 검사
	if (!IsKeyAndIVFileExist())
	{
		//없다면 AES키와 이니셜벡터를 /KEY 폴더에 생성
		GenAndStoreAESKeyAndIV();
	}
	// /KEY 폴더로부터 AES키와 이니셜 벡터 파일을 로드
	LoadKeyAndIV();


	string buff[5];
	string Base64String1 = AESEncrypteFileToBase64("1.jpg.PNG");
	string Base64String2 = AESEncrypteFileToBase64("2.jpg.PNG");
	string Base64String3 = AESEncrypteFileToBase64("3.jpg.PNG");
	string Base64String4 = AESEncrypteFileToBase64("4.jpg.PNG");
	string Base64String5 = AESEncrypteFileToBase64("5.jpeg.PNG");	
	
	HttpClient client("localhost:1111");

	json j;
	j["AESBase64File"] = Base64String5;
	j["RSAKey"] = "Client1";
	j["DataName"] = string("Data") + to_string(0);	

	cout << "POST UploadFile\n" << endl;
	auto req = client.request("POST", "/UploadFile", j.dump());
	cout << req->content.string() << endl;

	HttpClient client2("localhost:3333");
	json j1;
	j1["RSAKey"] = "Client1";
	j1["DataName"] = string("Data") + to_string(0);
	auto req2 = client2.request("POST", "/DownloadFile", j1.dump());
	
	vector<char> v = AESDecrypteDecodeBase64(req2->content.string());
	string s(v.begin(), v.end());
	std::ofstream out("Client1Data0_result.PNG", ios::binary);
	out << s;
	out.close();

	cout << "end" << endl;
	
	return 0;
}




