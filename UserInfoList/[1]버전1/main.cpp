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
using namespace CryptoPP;


#include "hash.hpp"
#include "Block.hpp"
#include "common.hpp"
#include "BlockChain.hpp"
#include "json.hh"
using json = nlohmann::json;

#include "userinfolist.hpp"


InvertibleRSAFunction privkey;
RSAFunction pubkey;
UserInfoListClass UserInfoList;

void GenKeyPair(void)
{
	//Generate private key and public key
	AutoSeededRandomPool rng;
	privkey.Initialize(rng, 512);
	RSAFunction _pubkey(privkey);
	pubkey = _pubkey;
}

int main()
{

	GenKeyPair();

	UserInfoList.AddUser("a");
	User& u = UserInfoList.FindUser("a");


	u.AddUserData("DATA1", UserData(1, 1, "127.0.0.1:8080", "127.0.0.1:8080"));


	u = UserInfoList.FindUser("a");
	UserData& b = u.FindData("DATA1");
	cout << b.BackUpFileAddress << b.BlockIndex << b.FileAddress << b.HashIndex <<  endl;

	cout << u.UserName << u.MiningCount << u.UsageCount <<   endl;
	u.MiningSuccessHandle();
	cout << u.UserName << u.MiningCount << u.UsageCount << endl;

	return 0;
}

