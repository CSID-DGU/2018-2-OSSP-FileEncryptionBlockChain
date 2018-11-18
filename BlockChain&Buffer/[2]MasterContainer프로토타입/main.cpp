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




MasterContainerClass MasterContainer;

int main()
{
	//제네시스 블록 출력
	//cout << MasterContainer.GetFileHash(0, 0) << endl;

	//채굴기 소유자 설정.
	MasterContainer.SetOwnerUser("User1");

	//채굴시키기 위해 파일 해쉬  삽입, 5개가 찰때마다 자동으로 채굴이 시작된다.
	for (int i = 1; i < 7; i++)
		MasterContainer.InsertFileHash(to_string(i));

	//채굴 중지 (이로인해  블럭하나는 채굴하다가 취소된다.)
	//MasterContainer.m_MiningCancelFlag = true;

	//채굴이 완료될때 까지 3초 대기
	std::this_thread::sleep_for(3s);

	cout << MasterContainer.GetFileHash(1, 0) << endl;
	cout << MasterContainer.GetFileHash(1, 1) << endl;
	cout << MasterContainer.GetFileHash(1, 2) << endl;
	cout << MasterContainer.GetFileHash(1, 3) << endl;
	cout << MasterContainer.GetFileHash(1, 4) << endl;
	cout << MasterContainer.GetFileHash(2, 0) << endl;

	//1000초 대기
	std::this_thread::sleep_for(1000s);
	
	//범위 에러가 나는 샘플
	//cout << MasterContainer.GetFileHash(20, 4) << endl;

	return 0;
}

