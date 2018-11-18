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
		//�׻� ���۴� �ּ� �� ���� �غ�
		this->List.push_back(FileHashBuffer());
	}


	// ���� �ؽ��� �־��µ� 5���� �� á���� true ����
	bool InsertFileHash(string FileHash)
	{
		//�׻� ���۴� �ּ� �� ���� �غ�
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
		//BufferList ���� �����´�.
		if (BlockIndex > (m_BlockChain.getNumOfBlocks()-1))
		{
			BlockIndex = BlockIndex - m_BlockChain.getNumOfBlocks();
			return m_HashBufferList.GetFileHash(BlockIndex, HashIndex);
		}
		else
		{
			//��ü�ο��� �����´�.
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
		//�ְ� ���ۿ� 5���� á����?
		if(true == m_HashBufferList.InsertFileHash(FileHash))
		{

			//ä������ �ƴ϶�� �ٷ� ä������
			if (m_MiningFlag == false)
			{				
				//ä���� �� it�� ������� ä�� ������ ����.
				thread t1([&]() { 
					// it �� ä���ؾ� �ϴ� ��
					auto it = m_HashBufferList.GetLastBufferBlockIt();
					MiningThreadFunc(it);
					//cout << "test" << endl;
				});
				t1.detach();
								
				m_MiningFlag = true;
			}
			//�̹� ä�����̶�� ť�� ����.
			else
			{
				auto it = m_HashBufferList.GetLastBufferBlockIt();
				m_MiningQueue.push(it);
			}
		}		
	}

	void SetOwnerUser(string OwnerUser) { this->m_OwnerUser = OwnerUser; }

	//ä�� ������� ä�� �Լ�.
	pair<string, string> MyFindHash(int index, string prevHash, vector<string> &merkle, bool& flag) {
		string header = to_string(index) + prevHash + getMerkleRoot(merkle);
		unsigned int nonce;
		for (nonce = 0; nonce < 0xFFFFFFFF; nonce++) {

			//���� �÷��װ� �����Ǹ� ��� ä�� �ߴ�
			if (flag == true) { flag = false;  return make_pair("fail", "fail"); }

			string blockHash = sha256(header + to_string(nonce));

			//20��Ʈ 0���� ���߱�, 10������ �ɸ�.
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
 * @param it ä���� ���� �ݺ���.
 */
void MasterContainerClass::MiningThreadFunc(list<FileHashBuffer>::iterator it)
{
	cout << "Mining start " << endl;

	//ä�� 
	auto pair = this->MyFindHash(m_BlockChain.getNumOfBlocks(), m_BlockChain.getLatestBlockHash(), 
		it->GetFileHashVector(), this->m_MiningCancelFlag);
	
	//ä�� �ߴ� ��û ->  ä������ ���۸� ������ ������ ����.
	if (pair == make_pair(string("fail"), string("fail"))) 
	{ 
		// �ٸ������� ä���� �Ϸ��ؼ� ���� �����״� it�� ����Ʈ���� ����
		//m_HashBufferList.EraseBufferBlock(it);
		//��ó��
		//�ٸ� ���� ä���� ��� �� �� �ֵ��� �÷��� ����
		
		this->m_MiningCancelFlag = false;

		//ť�� �̾ ä���ؾ��� ���� �ִٸ� ä������.
		if (!m_MiningQueue.empty())
		{
			///ä�� ������ ����
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

	//ä���� �Ϸ��� ���۴� ����,
	m_HashBufferList.EraseBufferBlock(it);

	if (this->m_OwnerUser != "")
	{
		//����� ���� ���.
	}

	// send the blockchain to the network
	//sendNewChain(&listOfNodes, bc.toJSON());



	//ť�� �̾ ä���ؾ��� ���� �ִٸ� ä������.
	if (!m_MiningQueue.empty())
	{
		///ä�� ������ ����
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