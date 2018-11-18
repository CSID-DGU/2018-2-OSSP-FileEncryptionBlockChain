// LocalTest1.cpp : �� ���Ͽ��� 'main' �Լ��� ���Ե˴ϴ�. �ű⼭ ���α׷� ������ ���۵ǰ� ����˴ϴ�.
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
	//���׽ý� ��� ���
	//cout << MasterContainer.GetFileHash(0, 0) << endl;

	//ä���� ������ ����.
	MasterContainer.SetOwnerUser("User1");

	//ä����Ű�� ���� ���� �ؽ�  ����, 5���� �������� �ڵ����� ä���� ���۵ȴ�.
	for (int i = 1; i < 7; i++)
		MasterContainer.InsertFileHash(to_string(i));

	//ä�� ���� (�̷�����  ���ϳ��� ä���ϴٰ� ��ҵȴ�.)
	//MasterContainer.m_MiningCancelFlag = true;

	//ä���� �Ϸ�ɶ� ���� 3�� ���
	std::this_thread::sleep_for(3s);

	cout << MasterContainer.GetFileHash(1, 0) << endl;
	cout << MasterContainer.GetFileHash(1, 1) << endl;
	cout << MasterContainer.GetFileHash(1, 2) << endl;
	cout << MasterContainer.GetFileHash(1, 3) << endl;
	cout << MasterContainer.GetFileHash(1, 4) << endl;
	cout << MasterContainer.GetFileHash(2, 0) << endl;

	//1000�� ���
	std::this_thread::sleep_for(1000s);
	
	//���� ������ ���� ����
	//cout << MasterContainer.GetFileHash(20, 4) << endl;

	return 0;
}

