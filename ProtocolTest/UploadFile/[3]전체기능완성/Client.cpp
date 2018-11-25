
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
	string Base64String1 = AESEncrypteFileToBase64("1.jpg");
	string Base64String2 = AESEncrypteFileToBase64("2.jpg");
	string Base64String3 = AESEncrypteFileToBase64("3.jpg");
	string Base64String4 = AESEncrypteFileToBase64("4.jpg");
	string Base64String5 = AESEncrypteFileToBase64("5.jpeg");
	string Base64String6 = AESEncrypteFileToBase64("6.jpeg");
	 buff[0] = Base64String1;
	 buff[1] = Base64String2;
	 buff[2] = Base64String3;
	 buff[3] = Base64String4;
	 buff[4] = Base64String5;


	HttpClient client("localhost:3333");

	for (int i = 0; i < 10; i++)
	{
		try
		{	
			std::this_thread::sleep_for(3s);
			json j;
			j["AESBase64File"] = Base64String5;
			j["RSAKey"] = "Client1";
			j["DataName"] = string("Data")+to_string(i);

			cout << "POST UploadFile\n" << endl;
			auto req = client.request("POST", "/UploadFile", j.dump());
			cout << req->content.string() << endl;
		}
		catch (const SimpleWeb::system_error &e)
		{
		cerr << "Client request error: " << e.what() << endl;
		}
	}	

	
	return 0;
}




