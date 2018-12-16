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

	json toJSON()
	{
		json retJSON;
		retJSON["UserNum"] = _UserInfoList.size();

		for (int i = 0; i < _UserInfoList.size(); i++)
		{
			retJSON[to_string(i)]["UserName"] = _UserInfoList[i].UserName;
			retJSON[to_string(i)]["MiningCount"] = _UserInfoList[i].MiningCount;
			retJSON[to_string(i)]["UsageCount"] = _UserInfoList[i].UsageCount;
			if (_UserInfoList[i].UserDataMap.size() != 0)
			{
				int DataNum = _UserInfoList[i].UserDataMap.size();
				retJSON[to_string(i)]["DataNum"] = DataNum;

				int j = 0;
				for (auto iter = _UserInfoList[i].UserDataMap.begin();
					iter != _UserInfoList[i].UserDataMap.end(); iter++)
				{
					retJSON[to_string(i)][to_string(j)]["first"] = iter->first;
					retJSON[to_string(i)][to_string(j)]["second"]["BlockIndex"] = iter->second.BlockIndex;
					retJSON[to_string(i)][to_string(j)]["second"]["HashIndex"] = iter->second.HashIndex;
					retJSON[to_string(i)][to_string(j)]["second"]["FileAddress"] = iter->second.FileAddress;
					retJSON[to_string(i)][to_string(j)]["second"]["BackUpFileAddress"] = iter->second.BackUpFileAddress;
					j++;
				}
			}
			else {
				retJSON[to_string(i)]["DataNum"] = 0;
			}
		}

		return retJSON;
	}

	bool ReBuildUserInfoList(json UserInfoJSON)
	{
		int UserNum = UserInfoJSON["UserNum"].get<int>();
		for (int i = 0; i < UserNum; i++)
		{
			string UserName = UserInfoJSON[to_string(i)]["UserName"].get<string>();
			AddUser(UserName);
			User& u = FindUser(UserName);
			u.MiningCount = UserInfoJSON[to_string(i)]["MiningCount"].get<int>();
			u.UsageCount = UserInfoJSON[to_string(i)]["UsageCount"].get<int>();

			int DataNum = UserInfoJSON[to_string(i)]["DataNum"].get<int>();
			if (DataNum != 0)
			{
				for (int j = 0; j < DataNum; j++)
				{
					string DataName = UserInfoJSON[to_string(i)][to_string(j)]["first"].get<string>();
					int BlockIndex = UserInfoJSON[to_string(i)][to_string(j)]["second"]["BlockIndex"].get<int>();
					int HashIndex = UserInfoJSON[to_string(i)][to_string(j)]["second"]["HashIndex"].get<int>();
					string FileAddress = UserInfoJSON[to_string(i)][to_string(j)]["second"]["FileAddress"].get<string>();
					string BackUpFileAddress = UserInfoJSON[to_string(i)][to_string(j)]["second"]["BackUpFileAddress"].get<string>();

					u.AddUserData(DataName, UserData(BlockIndex, HashIndex, FileAddress, BackUpFileAddress));
				}
			}
		}

		return true;
	}
};