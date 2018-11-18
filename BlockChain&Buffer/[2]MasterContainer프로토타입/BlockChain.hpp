//author: tko
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <iostream>
#include <string>
#include "hash.hpp"
#include <vector>
#include <memory>
#include <stdexcept>
#include <queue>

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
private:
	vector<unique_ptr<Block> > blockchain; //vector that is the blockchain
};

// If integer passed into constructor is 0, it the first node and creates the genesis block
BlockChain::BlockChain(int genesis) {
	if (genesis == 0) {
		vector<string> v;
		v.push_back("Genesis Block!");
		// string header = to_string(0) + string("00000000000000") + getMerkleRoot(v);
		auto hash_nonce_pair = findHash(0, string("00000000000000"), v);
		this->blockchain.push_back(std::make_unique<Block>(0, string("00000000000000"), hash_nonce_pair.first, hash_nonce_pair.second, v));
		printf("Created blockchain!\n");
	}
}
// Gets block based on the index
Block BlockChain::getBlock(int index) {
	for (int i = 0; i < blockchain.size(); i++) {
		if (blockchain[i]->getIndex() == index) {
			return *(blockchain[i]);
		}
	}
	throw invalid_argument("Index does not exist.");
}

// returns number of blocks
int BlockChain::getNumOfBlocks(void) {
	return this->blockchain.size();
}

// checks whether data fits with the right hash -> add block
int BlockChain::addBlock(int index, string prevHash, string hash, string nonce, vector<string> &merkle) {
	string header = to_string(index) + prevHash + getMerkleRoot(merkle) + nonce;
	if ((!sha256(header).compare(hash)) && (hash.substr(0, 2) == "00") && (index == blockchain.size())) {
		printf("Block hashes match --- Adding Block %s \n", hash.c_str());
		this->blockchain.push_back(std::make_unique<Block>(index, prevHash, hash, nonce, merkle));
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
		j["data"][this->blockchain[i]->getIndex()] = this->blockchain[i]->toJSON();
	}
	return j.dump(3);
}

// replaces Chain with new chain represented by a JSON, used when node sends new blockchain
int BlockChain::replaceChain(json chain) {
	//remove all blocks except for the first block
	while (this->blockchain.size() > 1) {
		this->blockchain.pop_back();
	}
	for (int a = 1; a < chain["length"].get<int>(); a++) {
		auto block = chain["data"][a];
		vector<string> data = block["data"].get<vector<string> >();
		this->addBlock(block["index"], block["previousHash"], block["hash"], block["nonce"], data);
	}
	return 1;
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


	// 파일 해쉬를 넣었는데 5개가 다 찼으면 true 리턴
	bool InsertFileHash(string FileHash)
	{
		//항상 버퍼는 최소 한 블럭씩 준비
		if(List.empty()) this->List.push_back(FileHashBuffer());

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
		list<FileHashBuffer>::iterator iter = this->List.end();
		iter--;
		return iter;
	}

	int GetBufferBlockNumber()
	{
		return this->List.size();
	}




};


class MasterContainerClass
{
private:
	BlockChain m_BlockChain;
	FileHashBufferList m_HashBufferList;
	string m_OwnerUser;
	bool m_MiningFlag;
	queue< list<FileHashBuffer>::iterator > m_MiningQueue;


	void MiningThreadFunc(list<FileHashBuffer>::iterator it);
	


public:
	bool m_MiningCancelFlag;


	//Constructor
	MasterContainerClass() : m_BlockChain(0), m_HashBufferList(), m_MiningCancelFlag(false),
							m_OwnerUser(""), m_MiningFlag(false), m_MiningQueue() {}

	string GetFileHash(int BlockIndex, int HashIndex )
	{
		//BufferList 에서 가져온다.
		if (BlockIndex > (m_BlockChain.getNumOfBlocks()-1))
		{
			BlockIndex = BlockIndex - m_BlockChain.getNumOfBlocks();
			return m_HashBufferList.GetFileHash(BlockIndex, HashIndex);
		}
		else
		{
			//블럭체인에서 가져온다.
			try {
				return m_BlockChain.getBlock(BlockIndex).GetFileHash(HashIndex);
			}
			catch (const std::invalid_argument& ia) {
				std::cerr << "Invalid argument: " << ia.what() << '\n';
				exit(-1);
			}
		}
	}

	void InsertFileHash(string FileHash)
	{
		//넣고 버퍼에 5개가 찼는지?
		if(true == m_HashBufferList.InsertFileHash(FileHash))
		{

			//채굴중이 아니라면 바로 채굴시작
			if (m_MiningFlag == false)
			{				
				//채굴할 블럭 it를 대상으로 채굴 쓰레드 생성.
				thread t1([&]() { 
					// it 는 채굴해야 하는 블럭
					auto it = m_HashBufferList.GetLastBufferBlockIt();
					MiningThreadFunc(it);
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
	pair<string, string> MyFindHash(int index, string prevHash, vector<string> &merkle, bool& flag) {
		string header = to_string(index) + prevHash + getMerkleRoot(merkle);
		unsigned int nonce;
		for (nonce = 0; nonce < 0xFFFFFFFF; nonce++) {

			//중지 플래그가 설정되면 즉시 채굴 중단
			if (flag == true) { flag = false;  return make_pair("fail", "fail"); }

			string blockHash = sha256(header + to_string(nonce));

			//20비트 0으로 맞추기, 10분정도 걸림.
			if (blockHash.substr(0, 5) == "00000") {
				// cout << "nonce: " << nonce;
				// cout << "header: " << header;
				return make_pair(blockHash, to_string(nonce));
				break;
			}
		}
		return make_pair("fail", "fail");
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
		it->GetFileHashVector(), this->m_MiningCancelFlag);
	
	//채굴 중단 요청 ->  채굴중인 버퍼를 버리고 쓰레드 종료.
	if (pair == make_pair(string("fail"), string("fail"))) 
	{ 
		// 다른곳에서 채굴을 완료해서 블럭을 줬을테니 it는 리스트에서 제거
		//m_HashBufferList.EraseBufferBlock(it);
		//후처리
		//다른 블럭의 채굴이 계속 될 수 있도록 플래그 제거
		
		this->m_MiningCancelFlag = false;

		//큐에 이어서 채굴해야할 블럭이 있다면 채굴진행.
		if (!m_MiningQueue.empty())
		{
			///채굴 쓰레드 생성
			thread t1([&]() { MiningThreadFunc(m_MiningQueue.front()); });
			t1.detach();
			m_MiningQueue.pop();
		}
		else
			m_MiningFlag = false;

		cout << "Mining end " << endl;
		return; 
	}


	// add the block to the blockchain
	m_BlockChain.addBlock(m_BlockChain.getNumOfBlocks(), m_BlockChain.getLatestBlockHash(), pair.first, pair.second, 
		it->GetFileHashVector());

	//채굴을 완료한 버퍼는 삭제,
	m_HashBufferList.EraseBufferBlock(it);

	if (this->m_OwnerUser != "")
	{
		//사용자 권한 상승.
	}

	// send the blockchain to the network
	//sendNewChain(&listOfNodes, bc.toJSON());



	//큐에 이어서 채굴해야할 블럭이 있다면 채굴진행.
	if (!m_MiningQueue.empty())
	{
		///채굴 쓰레드 생성
		thread t1([&]() { MiningThreadFunc(m_MiningQueue.front()); });
		t1.detach();
		m_MiningQueue.pop();
	}
	else
		m_MiningFlag = false;


	cout << "Mining end " << endl;
	return ;
	
}


#endif