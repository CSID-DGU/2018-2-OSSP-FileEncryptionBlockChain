//2018-10-28 ±Ë»÷∞« : UserData, User, UserInfoListClass øœº∫


#pragma once
#include <vector>
#include <string>
#include <map>
using namespace std;

class UserData
{
public:
	int BlockIndex;
	int HashIndex;
	string FileAddress;
	string BackUpFileAddress;
	

	UserData(int _BlockIndex, int _HashIndex, string _FileAddress, string _BackUpFileAddress)
	{
		this->BlockIndex = _BlockIndex;
		this->HashIndex = _HashIndex;
		this->FileAddress = _FileAddress;
		this->BackUpFileAddress = _BackUpFileAddress;
	}
};

class User
{
public:
	string UserName;
	map<string, UserData> UserDataMap;
	int MiningCount;
	int UsageCount;
	
	User()
	{
		this->UserName = "";
		this->MiningCount = 0;
		this->UsageCount = 5;
	}
	User(string _UserName)
	{
		this->UserName = _UserName;
		this->MiningCount = 0;
		this->UsageCount = 5;
	}

	void MiningSuccessHandle()
	{
		this->MiningCount++;
		this->UsageCount += 5;
	}

	bool IsExistData(string DataName)
	{
		if (this->UserDataMap.count(DataName) != 0) {
			return true;
		}
		else {
			return false;
		}
	}
	bool IsExhaustedCount()
	{
		if (this->UsageCount == 0) return true;
		else return false;
	}

	void DecrementUsageCount()
	{
		this->MiningCount--;
	}



	bool AddUserData(string DataName, UserData DataInfo)
	{
		if (this->UserName == "")
		{
			cout << "this User Data has Not initialize public key" << endl;
			return false;
		}

		auto it = this->UserDataMap.find(DataName);
		if (it != UserDataMap.end())
		{
			cout << "this Data Name already exist" << endl;
			return false;
		}

		this->UserDataMap.insert(make_pair(DataName, DataInfo));
		return true;
	}

	UserData& FindData(string DataName)
	{
		return this->UserDataMap.find(DataName)->second;
	}

};



class UserInfoListClass
{
public:
	vector<User> _UserInfoList;

	bool AddUser(string _UserName)
	{
		for (auto it = this->_UserInfoList.begin(); it != this->_UserInfoList.end(); it++)
		{
			if (it->UserName == _UserName)
			{
				cout << "This user is already registered" << endl;
				return false;
			}
		}
		_UserInfoList.push_back(User(_UserName));
		return true;
	}

	User& FindUser(string _UserName)
	{
		for (auto a = this->_UserInfoList.begin(); a != this->_UserInfoList.end(); a++)
		{
			if (a->UserName == _UserName)
			{
				return *a;
			}			
		}
		cout << "This user is not registered" << endl;
	}
};