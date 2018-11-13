//2018-10-28 ±Ë»÷∞« : UserData, User, UserInfoListClass øœº∫



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
	
	User()
	{
		UserName = "";
		MiningCount = 0;
	}
	User(string _UserName)
	{
		this->UserName = _UserName;
		MiningCount = 0;
	}


	void AddUserData(string DataName, UserData DataInfo)
	{
		if (this->UserName == "")
			cout << "this User Data has Not initialize public key" << endl;

		this->UserDataMap.insert(make_pair(DataName, DataInfo));
	}

	UserData& FindData(string DataName)
	{
		for (auto it = this->UserDataMap.begin(); it != this->UserDataMap.end(); it++)
		{
			if (it->first == DataName)
			{
				return it->second;
			}
			cout << "This Data is not exist" << endl;
		}
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