// LocalTest1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//


#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/aes.h>
#include <crypto++/modes.h>
using namespace CryptoPP;


#include "hash.hpp"
#include "Block.hpp"
#include "common.hpp"
#include "BlockChain.hpp"
#include "json.hh"
using json = nlohmann::json;

#include "userinfolist.hpp"
#include "AES.hpp"



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
	
	
	//testfile1.txt 파일을 AES로 암호화 한후 Base64로 인코딩한후 string 형태로 리턴
	string Base64String = AESEncrypteFileToBase64("json.hh");


	//위의 결과를 output.txt 파일에 출력
	std::ofstream out("output.txt");
	out << Base64String;
	out.close();

	//Base64형태로 인코딩된 string을 Base64로 디코딩함, 결과물은 AES로 암호화된 형태
	string DecodeBase64string = DecodeBase64String(Base64String);

	// AES로 암호화된 string을 sha256으로 해쉬,
	cout << sha256(DecodeBase64string) << endl;

	//AES 암호화 + Base64로 인코딩된 string을 Base64디코딩 + AES복호화를 통해 원문으로 복구 
	 std::vector<char> Buffer = AESDecrypteDecodeBase64(Base64String);
	//cout << plaintext << endl;


	 //원본 파일 restore
	 std::ofstream out2("output.txt", std::ofstream::binary);
	 out2.write((char *)&Buffer[0], Buffer.capacity());
	 out2.close();


	return 0;
}


