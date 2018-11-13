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


FileHashBufferList HashBufferList;

int main()
{
	for (int i = 1; i < 7; i++)
	{
		// 파일 해쉬값을 넣고 넣었는데 5개가 다 차면 true를 리턴
		if (HashBufferList.InsertFileHash(to_string(i)))
		{
			cout << "Full Five Hash" << " index : " << i << endl;
		}
	}

	// 0번 버퍼의 i번째 해쉬값을 가져옴
	for (int i = 0; i < 5; i++)
		cout << HashBufferList.GetFileHash(0, i) << endl;

	for (int i = 0; i < 1; i++)
		cout << HashBufferList.GetFileHash(1, i) << endl;

	//유효하지 않은 위치의 해쉬값을 가져오면 에러코드 발생
	//cout << HashBufferList.GetFileHash(1, 1) << endl;

	// 유효하지 않은 위치의 해쉬값을 가져오면 에러코드 발생
	//cout << HashBufferList.GetFileHash(2, 1) << endl;

	HashBufferList.ListDump();
	HashBufferList.EraseHeadBuffer();
	HashBufferList.ListDump();

	return 0;
}

