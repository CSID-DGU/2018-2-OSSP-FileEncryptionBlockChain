//author: tko
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H


#include <experimental/filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include "hash.hpp"
#include <vector>
#include <memory>
#include <stdexcept>
#include <queue>
#include <crypto++/osrng.h>

#include "userinfolist.hpp"
#include "json.hh"
using json = nlohmann::json;

class BlockChain {
public:
	BlockChain(int genesis = 1);
	Block getBlock(int index);
	// getBlock(string hash); //not implemented
	int getNumOfBlocks(void);
	int addBlock(int index, string prevHash, string hash, string nonce, vector<string> &merkle);
	string getLatestBlockHash(void);
	// void toString(void);
	string toJSON(void);
	int replaceChain(json chain);
	json HandleBlockFilesTransfer(json Parameter);
	bool ReplaceBlockFiles(map<string, string> *NodeInfoList, string MyNodeName);
	int m_BlockNum;	//파일까지 고려한 모든 블록 수
	const int m_MemoryBlockNumMax; // 메모리에 저장할 블록 수
	int replaceChain2(json chain); // 블록 하나는 남기고 나머지 블록만 추가

	vector<unique_ptr<Block> > blockchain; //vector that is the blockchain



};

// If integer passed into constructor is 0, it the first node and creates the genesis block
BlockChain::BlockChain(int genesis) : m_MemoryBlockNumMax(3) {
	if (genesis == 0) {
		vector<string> v;
		v.push_back("Genesis Block!");
		// string header = to_string(0) + string("00000000000000") + getMerkleRoot(v);
		auto hash_nonce_pair = findHash(0, string("00000000000000"), v);
		this->blockchain.push_back(std::make_unique<Block>(0, string("00000000000000"), hash_nonce_pair.first, hash_nonce_pair.second, v));
		printf("Created blockchain!\n");
		this->m_BlockNum = 1;
	}
}
// Gets block based on the index
Block BlockChain::getBlock(int index) {
	//메모리에서 가져오기
	if (index > m_BlockNum - m_MemoryBlockNumMax - 1) {
		for (int i = 0; i < blockchain.size(); i++) {
			if (blockchain[i]->getIndex() == index) {
				return *(blockchain[i]);
			}
		}
	}
	else { //파일에서 가져오기
		if (index == 0) //제네시스 블록 가져오기
		{
			ifstream in(string("BlockChainFiles//") + "Block" + to_string(index));
			int Index;
			string PreviousHash;
			string Hash;
			string Nonce;
			in >> Index >> PreviousHash >> Hash >> Nonce;


			vector<string> DataVector;
			DataVector.push_back("Genesis Block!");

			return Block(Index, PreviousHash, Hash, Nonce, DataVector);
		}
		//메모리에서 가져오기
		ifstream in(string("BlockChainFiles//") + "Block" + to_string(index));
		int Index;
		string PreviousHash;
		string Hash;
		string Nonce;
		string FileHash1;
		string FileHash2;
		string FileHash3;
		string FileHash4;
		string FileHash5;
		in >> Index >> PreviousHash >> Hash >> Nonce >>
			FileHash1 >> FileHash2 >> FileHash3 >> FileHash4 >> FileHash5;

		vector<string> FileHashVec;
		FileHashVec.push_back(FileHash1);
		FileHashVec.push_back(FileHash2);
		FileHashVec.push_back(FileHash3);
		FileHashVec.push_back(FileHash4);
		FileHashVec.push_back(FileHash5);

		return Block(Index, PreviousHash, Hash, Nonce, FileHashVec);
	}

	throw invalid_argument("Index does not exist.");
}

// returns number of blocks
int BlockChain::getNumOfBlocks(void) {
	//return this->blockchain.size();
	return this->m_BlockNum;
}

// checks whether data fits with the right hash -> add block
int BlockChain::addBlock(int index, string prevHash, string hash, string nonce, vector<string> &merkle) {
	string header = to_string(index) + prevHash + getMerkleRoot(merkle) + nonce;



	if ((!sha256(header).compare(hash)) && (hash.substr(0, 2) == "00")) {
		printf("Block hashes match --- Adding Block %s \n", hash.c_str());
		this->blockchain.push_back(std::make_unique<Block>(index, prevHash, hash, nonce, merkle));

		// 일정량 이상 차면 파일에 저장.
		if (this->blockchain.size() > this->m_MemoryBlockNumMax)
		{
			//./BlockChainFiles 디렉터리 생성
			std::experimental::filesystem::path dir("BlockChainFiles");
			if (!(experimental::filesystem::exists(dir))) {
				experimental::filesystem::create_directory(dir);
			}
			ofstream out(string("BlockChainFiles/") + "Block" + to_string(m_BlockNum - m_MemoryBlockNumMax - 1));

			out << blockchain[0]->getIndex() << endl;
			out << blockchain[0]->getPreviousHash() << endl;
			out << blockchain[0]->getHash() << endl;
			out << blockchain[0]->getNonce() << endl;
			for (int i = 0; i < 5; i++) {
				//제네시스 블록 예외처리
				if (blockchain[0]->getData()[0] == "Genesis Block!")
				{
					out << "Genesis Block!" << endl;
					break;
				}
				out << blockchain[0]->GetFileHash(i) << endl;
			}
			this->blockchain.erase(blockchain.begin());
		}
		return 1;
	}
	cout << "Hash doesn't match criteria\n";
	return 0;
}

// returns hash of the latest block, used for finding the previousHash when mining new block
string BlockChain::getLatestBlockHash(void) {
	return this->blockchain[blockchain.size() - 1]->getHash();
}

// returns JSON string of JSON - used to send to network
string BlockChain::toJSON() {
	json j;
	j["length"] = this->blockchain.size();
	for (int i = 0; i < this->blockchain.size(); i++) {
		j["data"][i] = this->blockchain[i]->toJSON();
	}
	return j.dump(3);
}

// replaces Chain with new chain represented by a JSON, used when node sends new blockchain
int BlockChain::replaceChain(json chain) {
	//그냥 모든 체인 삭제
	this->blockchain.clear();

	for (int a = 0; a < chain["length"].get<int>(); a++) {
		auto block = chain["data"][a];
		vector<string> data = block["data"].get<vector<string> >();
		this->addBlock(block["index"], block["previousHash"], block["hash"], block["nonce"], data);
	}
	return 1;
}

int BlockChain::replaceChain2(json chain) {

	//하나만 남기고 블록체인 삭제
	while (blockchain.size() != 1) {
		blockchain.pop_back();
	}

	for (int a = 0; a < chain["length"].get<int>(); a++) {
		auto block = chain["data"][a];
		vector<string> data = block["data"].get<vector<string> >();
		this->addBlock(block["index"], block["previousHash"], block["hash"], block["nonce"], data);
	}
	return 1;
}

json BlockChain::HandleBlockFilesTransfer(json Parameter)
{
	json retBlockFiles;
	int FirstBlockIndex = Parameter["FirtstBlockIndex"].get<int>();
	int FileBlockNum = getNumOfBlocks() - blockchain.size();
	vector<int> IndexVector;

	//디스크에 없는 파일 요청 예외처리
	if (FirstBlockIndex + 1 > FileBlockNum)
	{
		retBlockFiles["IsSuccess"] = false;
		return retBlockFiles;
	}

	//파일을 일정 단위로 읽어와서 json에 저장
	for (int BlockIndex = FirstBlockIndex; BlockIndex < BlockIndex + m_MemoryBlockNumMax; BlockIndex++)
	{
		if (BlockIndex + 1 > FileBlockNum)
			break;

		retBlockFiles[to_string(BlockIndex)] = getBlock(BlockIndex).toJSON();
		IndexVector.push_back(BlockIndex);
	}
	retBlockFiles["IndexVector"] = IndexVector;
	retBlockFiles["IsSuccess"] = true;

	//새로 채굴된 블럭이 있을수 있으니 다시 계산
	FileBlockNum = getNumOfBlocks() - blockchain.size();

	//더 보낼 파일이 있는경우
	if (IndexVector.back() < FileBlockNum - 1)
	{
		retBlockFiles["isFull"] = false;
	}
	//모든 파일을 전부 전송한 경우.
	else
	{
		retBlockFiles["isFull"] = true;
	}

	return retBlockFiles;
}

bool BlockChain::ReplaceBlockFiles(map<string, string> *NodeInfoList, string MyNodeName)
{
	auto it1 = NodeInfoList->find(MyNodeName);
	if (++it1 == NodeInfoList->end()) {
		it1 = NodeInfoList->begin();
	}
	string RequestNodeAddress = it1->second;


	int FirstFileIndex = 0;

	while (1)
	{

		HttpClient Client(RequestNodeAddress);
		json j;
		j["FirtstBlockIndex"] = FirstFileIndex;
		auto req = Client.request("GET", "/GETBlockFiles", j.dump());

		json ret = json::parse(req->content.string());

		//요청 failed
		if (ret["IsSuccess"] == false)
		{
			return false;
		}

		vector<int> IndexVector = ret["IndexVector"].get< vector<int> >();
		for (auto it = IndexVector.begin(); it != IndexVector.end(); it++)
		{
			//블록체인 디렉터리 생성
			std::experimental::filesystem::path dir("BlockChainFiles");
			if (!(experimental::filesystem::exists(dir))) {
				experimental::filesystem::create_directory(dir);
			}
			ofstream out(string("BlockChainFiles/") + "Block" + to_string(*it));

			//블록 읽어와서 파일에 쓰기
			auto block = ret[to_string(*it)];
			vector<string> data = block["data"].get<vector<string> >();
			out << block["index"].get<int>() << endl;
			out << block["previousHash"].get<string>() << endl;
			out << block["hash"].get<string>() << endl;
			out << block["nonce"].get<string>() << endl;
			for (int i = 0; i < 5; i++) {
				//제네시스 블록 예외처리
				if (*it == 0)
				{
					out << "Genesis Block!" << endl;
					break;
				}
				out << data[i] << endl;
			}
			cout << " Repace File" << *it << endl;
		}

		if (ret["isfull"] == true) {
			break;
		}
		else {
			FirstFileIndex = IndexVector.back() + 1;
			continue;
		}
	}

	return true;
}

class FileHashBufferList
{
public:
	list<FileHashBuffer> List;


	FileHashBufferList()
	{
		//항상 버퍼는 최소 한 블럭씩 준비
		this->List.push_back(FileHashBuffer());
	}

	bool IsEmpty()
	{
		this->List.empty();
	}

	// 파일 해쉬를 넣었는데 5개가 다 찼으면 true 리턴
	bool InsertFileHash(string FileHash)
	{
		//항상 버퍼는 최소 한 블럭씩 준비
		if (List.empty()) this->List.push_back(FileHashBuffer());

		if (this->List.back().IsFull())
		{
			List.push_back(FileHashBuffer(FileHash));
			return false;
		}
		else
		{
			List.back().InsertFileHash(FileHash);
			if (List.back().IsFull())
			{
				return true;
			}
			else false;
		}
	}

	string GetFileHash(int BufferIndex, int HashIndex)
	{
		assert(BufferIndex < List.size());

		auto it = List.begin();
		for (int i = 0; i < BufferIndex; i++) it++;
		return it->GetFileHash(HashIndex);
	}

	void ListDump()
	{
		int index = 0;
		for (auto it = this->List.begin(); it != this->List.end(); it++)
		{
			cout << "[" + to_string(index) + "]" + ' ';
			it->FileHashBufferDump();
			cout << endl;
			index++;
		}
	}

	void EraseBufferBlock(list<FileHashBuffer>::iterator it)
	{
		List.erase(it);
	}

	FileHashBuffer& GetBufferBlock(int BufferBlockIndex)
	{
		assert(BufferBlockIndex < List.size());

		auto it = List.begin();
		for (int i = 0; i < BufferBlockIndex; i++) it++;
		return *it;
	}

	list<FileHashBuffer>::iterator GetLastBufferBlockIt()
	{
		std::list<FileHashBuffer>::iterator iter = std::prev(this->List.end());
		return iter;
	}

	int GetBufferBlockNumber()
	{
		return this->List.size();
	}

	json toJSON()
	{
		int i = 0;
		json j;
		j["length"] = GetBufferBlockNumber();
		for (auto it = List.begin(); it != List.end(); it++)
		{
			j["data"][i] = it->FileHashVector;
			i++;
		}
		return j;
	}

	void MakeListByJSON(json j)
	{
		int NumOfBlocks = j["length"].get<int>();
		for (int i = 0; i < NumOfBlocks; i++)
		{
			FileHashBuffer BufferBlock;
			BufferBlock.FileHashVector = j["data"][i].get<vector<string>>();
			BufferBlock.HashCount = BufferBlock.FileHashVector.size();
			this->List.push_back(BufferBlock);
		}
		this->List.pop_front();
	}

};


class MasterContainerClass
{
public:
	BlockChain m_BlockChain;
	FileHashBufferList m_HashBufferList;
	string m_OwnerUser;
	bool m_MiningFlag;
	queue< list<FileHashBuffer>::iterator > m_MiningQueue;
	int m_BlockCount;
	int m_HashCount;
	uint32_t m_MiningSeed;
	map<string, string> *m_pNodeInfoList;
	UserInfoListClass *m_pUserInfoList;
	string m_MyNodeName;
	bool m_MiningCancelFlag;

	void MiningThreadFunc(list<FileHashBuffer>::iterator it);



	//Constructor
	MasterContainerClass() : m_BlockChain(0), m_HashBufferList(), m_MiningCancelFlag(false),
		m_OwnerUser(""), m_MiningFlag(false), m_MiningQueue(), m_BlockCount(0),
		m_HashCount(0), m_pNodeInfoList(nullptr), m_MyNodeName(""), m_pUserInfoList(nullptr)
	{
		CryptoPP::AutoSeededRandomPool rng;
		rng.GenerateBlock((byte *)&m_MiningSeed, 4);
		cout << "Mining Seed Value : " << m_MiningSeed << endl;
	}

	//진행중인 채굴 중단.
	void CancelMining()
	{
		if (m_MiningFlag == true) { m_MiningCancelFlag = true; }
	}


	//블럭체인 전파하는데 필요
	void SetNodeInfoList(map<string, string> *pNodeInfoList, string MyNodeName)
	{
		this->m_pNodeInfoList = pNodeInfoList;
		this->m_MyNodeName = MyNodeName;
	}

	//채굴보상을 지급하기 위해 필요
	void SetUserInfoList(UserInfoListClass *pUserInfoList)
	{
		this->m_pUserInfoList = pUserInfoList;
	}


	string GetFileHash(int BlockIndex, int HashIndex)
	{
		//BufferList 에서 가져온다.
		if (BlockIndex > (m_BlockChain.getNumOfBlocks() - 1))
		{
			BlockIndex = BlockIndex - m_BlockChain.getNumOfBlocks();
			return m_HashBufferList.GetFileHash(BlockIndex, HashIndex);
		}
		//블럭체인에서 가져온다.
		else
		{
			bool isHashError; //블록체인 난스, 해쉬 에러 여부 검사
			string retFileHash; //리턴할 파일해쉬값

			try {
				retFileHash = m_BlockChain.getBlock(BlockIndex).GetFileHash(HashIndex);
			}
			catch (const std::invalid_argument& ia) {
				std::cerr << "Invalid argument: " << ia.what() << '\n';
				exit(-1);
			}

			//블록체인 검증 절차
			for (int i = BlockIndex; i < m_BlockChain.getNumOfBlocks(); i++)
			{
				string PrevBlockHash;
				Block CurrentBlock = m_BlockChain.getBlock(i);
				string nonce = CurrentBlock.getNonce();
				string merkle = getMerkleRoot(CurrentBlock.getData());

				if (i == 0) { //제네시스 블록
					PrevBlockHash = string("00000000000000");
				}
				else {
					PrevBlockHash = m_BlockChain.getBlock(i - 1).getHash();
				}

				string header = to_string(i) + PrevBlockHash + merkle;
				string BlockHash = sha256(header + nonce);


				//에러처리1 : "00"이 맞는지?
				if (BlockHash.substr(0, 2) != "00") {
					cout << "BLOCKCHAINERROR!!!!!!!!!" << endl;
					//파일 + 메모리 블럭체인 교체 수행
				}

				//에러처리2 : 다음 블록해쉬와 맞는지?
				if (i != m_BlockChain.getNumOfBlocks() - 1)
				{
					if (m_BlockChain.getBlock(i + 1).getPreviousHash() != BlockHash)
					{
						cout << "BLOCKCHAINERROR!!!!!!!!!" << endl;
						//파일 + 메모리 블럭체인 교체 수행
					}
				}
			}

			return retFileHash;


			/*
			//블럭체인에서 가져온다.
			try {
				return m_BlockChain.getBlock(BlockIndex).GetFileHash(HashIndex);
			}
			catch (const std::invalid_argument& ia) {
				std::cerr << "Invalid argument: " << ia.what() << '\n';
				exit(-1);
			}
			*/
		}
	}

	pair<int, int> GetCurBlockAndHashIndex()
	{
		int BlockIndex = m_BlockCount + 1;
		int HashIndex = m_HashCount - 1;
		return pair<int, int>(BlockIndex, HashIndex);
	}

	void InsertFileHash(string FileHash)
	{
		m_HashCount++;
		if (m_HashCount == 6) { m_BlockCount++;  m_HashCount = 1; }


		//넣고 버퍼에 5개가 찼는지?
		if (true == m_HashBufferList.InsertFileHash(FileHash))
		{
			//채굴중이 아니라면 바로 채굴시작
			if (m_MiningFlag == false)
			{
				//채굴할 블럭 it를 대상으로 채굴 쓰레드 생성.
				thread t1([&]() {
					// it 는 채굴해야 하는 블럭
					auto it1 = m_HashBufferList.GetLastBufferBlockIt();
					MiningThreadFunc(it1);
					//cout << "test" << endl;
				});
				t1.detach();

				m_MiningFlag = true;
			}
			//이미 채굴중이라면 큐에 넣음.
			else
			{
				auto it = m_HashBufferList.GetLastBufferBlockIt();
				m_MiningQueue.push(it);
			}
		}

	}

	void SetOwnerUser(string OwnerUser) { this->m_OwnerUser = OwnerUser; }

	//채굴 쓰레드용 채굴 함수.
	pair<string, string> MyFindHash(int index, string prevHash, vector<string> &merkle) {
		string header = to_string(index) + prevHash + getMerkleRoot(merkle);

		uint32_t nonce;
		for (nonce = this->m_MiningSeed; ; nonce++) {

			//중지 플래그가 설정되면 즉시 채굴 중단
			if (m_MiningCancelFlag == true) { m_MiningCancelFlag = false;  return make_pair("fail", "fail"); }

			string blockHash = sha256(header + to_string(nonce));

			//20비트 0으로 맞추기, 10분정도 걸림.
			if (blockHash.substr(0, 4) == "0000") {
				// cout << "nonce: " << nonce;
				// cout << "header: " << header;
				return make_pair(blockHash, to_string(nonce));
				break;
			}
		}
		return make_pair("fail", "fail");
	}

	//MasterContainer의 내용물을 json 형태로 출력
	json toJSON()
	{
		json j;
		j["blockChain"] = json::parse(m_BlockChain.toJSON());
		j["BufferList"] = m_HashBufferList.toJSON();
		j["BlockCount"] = m_BlockCount;
		j["HashCount"] = m_HashCount;

		return j;
	}



	//입력받은 json 기반으로 MasterContainer초기화
	void MakeContainerByJSON(json j, string MyNodeName, map<string, string> *pNodeInfoList, string OwnerUser = "")
	{
		if (OwnerUser != "") {
			m_OwnerUser = OwnerUser;
		}

		m_pNodeInfoList = pNodeInfoList;
		m_MyNodeName = MyNodeName;

		m_HashCount = j["HashCount"];
		m_BlockCount = j["BlockCount"];
		m_BlockChain.replaceChain(j["blockChain"]);
		m_HashBufferList.MakeListByJSON(j["BufferList"]);


		for (auto it = m_HashBufferList.List.begin(); it != m_HashBufferList.List.end(); ++it)
		{
			//버퍼가 꽉차서 채굴이 필요한 블럭발견.
			if (it->IsFull())
			{
				//채굴쓰레드 생성
				//채굴중이 아니라면 바로 채굴시작
				if (m_MiningFlag == false)
				{
					//채굴할 블럭 it를 대상으로 채굴 쓰레드 생성.
					thread t1([&]() {
						// it 는 채굴해야 하는 블럭
						MiningThreadFunc(it);
						//cout << "test" << endl;
					});
					t1.detach();

					m_MiningFlag = true;
				}
				//이미 채굴중이라면 큐에 넣음.
				else
				{
					m_MiningQueue.push(it);
				}
			}
			else
			{
				break;
			}
		}
	}



};

/**
 * @param it 채굴할 블럭의 반복자.
 */
void MasterContainerClass::MiningThreadFunc(list<FileHashBuffer>::iterator it)
{
	cout << "Mining start " << endl;

	//채굴 
	auto pair = this->MyFindHash(m_BlockChain.getNumOfBlocks(), m_BlockChain.getLatestBlockHash(),
		it->GetFileHashVector());

	//채굴 중단 요청 ->  채굴중인 버퍼를 버리고 쓰레드 종료.
	if (pair == make_pair(string("fail"), string("fail")))
	{
		// 다른곳에서 채굴을 완료해서 블럭을 줬을테니 it는 리스트에서 제거
		m_HashBufferList.EraseBufferBlock(it);
		//다른 블럭의 채굴이 계속 될 수 있도록 플래그 제거

		this->m_MiningCancelFlag = false;

		//큐에 이어서 채굴해야할 블럭이 있다면 채굴진행.
		if (!m_MiningQueue.empty())
		{
			///채굴 쓰레드 생성
			auto it = m_MiningQueue.front();
			m_MiningQueue.pop();
			thread t1([=]() { MiningThreadFunc(it); });
			t1.detach();
		}
		else
			m_MiningFlag = false;

		cout << "Migning Forced termination " << endl;
		return;
	}
	//Increase Block Num
	m_BlockChain.m_BlockNum++;



	// add the block to the blockchain
	m_BlockChain.addBlock(m_BlockChain.getNumOfBlocks() - 1, m_BlockChain.getLatestBlockHash(), pair.first, pair.second,
		it->GetFileHashVector());

	// send the blockchain to the network
	for (auto it = m_pNodeInfoList->begin(); it != m_pNodeInfoList->end(); it++)
	{
		if (it->first != m_MyNodeName)
		{
			HttpClient client(it->second);
			try {
				json j;
				if (m_BlockChain.getNumOfBlocks() > m_BlockChain.m_MemoryBlockNumMax) {
					j["ReplaceType"] = 2;
				}
				else {
					j["ReplaceType"] = 1;
				}

				j["BlockNum"] = m_BlockChain.getNumOfBlocks();
				j["BlockChain"] = m_BlockChain.toJSON();
				auto req = client.request("POST", "/newchain", j.dump());
				cout << "BlockChainSend" << endl;
			}
			catch (const SimpleWeb::system_error &e) {
				cerr << "Client request error: " << e.what() << endl;
			}
		}
	}

	//채굴을 완료한 버퍼는 삭제,
	m_HashBufferList.EraseBufferBlock(it);

	if (this->m_OwnerUser != "")
	{
		//채굴 성공 보상 사용자 권한 상승.
		User& u = m_pUserInfoList->FindUser(m_OwnerUser);
		u.MiningSuccessHandle();
	}



	//큐에 이어서 채굴해야할 블럭이 있다면 채굴진행.
	if (!m_MiningQueue.empty())
	{
		///채굴 쓰레드 생성
		auto it = m_MiningQueue.front();
		m_MiningQueue.pop();
		thread t1([=]() { MiningThreadFunc(it); });
		t1.detach();
	}
	else
		m_MiningFlag = false;


	cout << "Mining end " << endl;
	return;

}


#endif