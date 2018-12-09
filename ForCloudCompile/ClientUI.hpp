// ClientUI.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/filebox.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <map>

#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "requests.hpp"
#include "client_http.hpp"
#include "hash.hpp"
#include "json.hh"
#include "userinfolist.hpp"
#include "AES.hpp"
using namespace std;
using namespace nana;
using json = nlohmann::json;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

string MyName;
map<string, string> NodeInfoList;
map<string, string> FileNameTable;

int main()
{
	form fm;
	label lab{ fm, "<bold size=14> Test1, <blue>File Encryption Block Chain</>!</>" };
	lab.format(true);
	fm.caption("File Encryption Block Chain Client Program");
	fm.size(nana::size(500, 300));

	//���α׷� ����� FileNameTable ����
	fm.events().unload([](const arg_unload &arg)
	{
		std::ofstream ofs("KEY\\FileNameTable.dat");
		boost::archive::text_oarchive oa(ofs);
		oa << FileNameTable;
	});

	textbox TextBox(fm);
	TextBox.editable(false);
	
	textbox UploadDataEdit(fm);
	UploadDataEdit.multi_lines(false);
	
	textbox DownloadDataEdit(fm);
	DownloadDataEdit.multi_lines(false);

	textbox NodeAddressEdit(fm);
	NodeAddressEdit.multi_lines(false);



	//Ŭ���̾�Ʈ ��� ��ư
	button SignUpButton{ fm, "Sign up" };
	SignUpButton.events().click([&] {
		fm.close(); 
	//Add User ��û ����,
	
	});

	//���� ���ε� ��ư
	button UloadButton{ fm, "FileUpload" };
	UloadButton.events().click([&] {
		
		string NodeAddress;
		string DataName;
		UploadDataEdit.getline(0, DataName);
		NodeAddressEdit.getline(0, NodeAddress);
		if (DataName ==""|| NodeAddress =="")
		{
			msgbox mb(fm, "Error");
			mb << "You have to Input Data Name or Node Address";
			mb.show();
			return;
		}


		filebox fb(true);
		if (fb()) //����ڰ� ���� ����
		{
			string filePath = fb.file();
			TextBox.append(filePath + '\n', false);
			
			string Base64Encrypte = AESEncrypteFileToBase64(filePath);
			TextBox.append("* Encrypte File Success\n", false);

				HttpClient client(NodeAddress);
				json j;
				j["AESBase64File"] = Base64Encrypte;
				j["RSAKey"] = MyName;
				j["DataName"] = DataName ;

				
				try {
					TextBox.append("* POST Upload File\n", false);
					auto req = client.request("POST", "/UploadFile", j.dump());
					TextBox.append(req->content.string(), false);
				}
				catch (const SimpleWeb::system_error &e)
				{
					TextBox.append(string("* Client request error:") + e.what(), false);
				}
				
				/*
				//ä�� �׽�Ʈ�� ���ε� �ڵ� (�ѹ��� 5���� ���ε� ��û)
				try {
					for (int i = 0; i < 5; i++) {
						j["DataName"] = DataName + to_string(i);
						DataName = DataName + to_string(i);
						TextBox.append("* POST Upload File\n", false);
						auto req = client.request("POST", "/UploadFile", j.dump());
						TextBox.append(req->content.string(), false);
					}
				}
				catch (const SimpleWeb::system_error &e)
				{
					TextBox.append(string("* Client request error:") + e.what(), false);
				}
				*/

				
				//FileNameTable ���
				FileNameTable.insert(make_pair(DataName, filePath));
		}	
	});


	button DownloadButton{ fm, "Download File" };
	DownloadButton.events().click([&] {
		//fm.close();
		//FileUpload ��û����



		string NodeAddress;
		string DataName;
		DownloadDataEdit.getline(0, DataName);
		NodeAddressEdit.getline(0, NodeAddress);

		if (DataName == "" || NodeAddress == "")
		{
			msgbox mb(fm, "Error");
			mb << "You have to Input Data Name or Node Address";
			mb.show();
			return;
		}


		HttpClient client2(NodeAddress);
		json j1;
		j1["RSAKey"] = MyName;
		j1["DataName"] = DataName;

		TextBox.append("* GET DownloadFile File\n", false);
		auto req2 = client2.request("POST", "/DownloadFile", j1.dump());
		vector<char> v = AESDecrypteDecodeBase64(req2->content.string());
		TextBox.append(string("* Decrypte File Success File size") + '['+ to_string(v.size()) + "]\n", false);
		string s(v.begin(), v.end());

		std::ofstream out(FileNameTable[DataName], ios::binary);
		out << s;
		out.close();

	});


	button GetUserInfoButton{ fm, "Get user Info" };
	GetUserInfoButton.events().click([&] {
		//fm.close();
		//GetUserInfo ��û����

		string NodeAddress;
		NodeAddressEdit.getline(0, NodeAddress);

		if (NodeAddress == "")
		{
			msgbox mb(fm, "Error");
			mb << "You have to Input Node Address";
			mb.show();
			return;
		}

		json j1;
		j1["RSAKey"] = MyName;

		TextBox.append("* GET User Information\n", false);
		HttpClient client2(NodeAddress);
		auto req2 = client2.request("POST", "/RequestList", j1.dump());

		//������ ����Ʈ ���
		TextBox.append(req2->content.string(), false);	
	});


	fm.div("vert<weight= 10% <Caption> <weight = 30% NodeAddress>> \
		 <<weight = 70% p1> \
		< vert< vert<><weight=60% p2><> > \
		< vert<Edit1><weight=50% p3>> \
		< vert<Edit2><weight=50% p4>>	\
		< vert<><weight=60% p5><>>>>");


	fm["Caption"] << lab;
	fm["NodeAddress"] << NodeAddressEdit;
	fm["p1"] << TextBox;
	fm["p2"] << SignUpButton;
	fm["Edit1"] << UploadDataEdit;
	fm["p3"] << UloadButton;
	fm["Edit2"] << DownloadDataEdit;
	fm["p4"] << DownloadButton;
	fm["p5"] << GetUserInfoButton;
	fm.collocate();

	//�ʱ� �۾���
	// /KEY ������ AESŰ�� ��ȣȭ�� ���� �̴ϼȺ��� �ΰ��� ������ �ִ��� �˻�
	if (!IsEncryptionFileExist())
	{
		//���ٸ� AESŰ�� �̴ϼȺ��͸� /KEY ������ ����
		GenEncryptionFile();
		TextBox.append("* Make Encryption Materials\n", false);
	}
	// /KEY �����κ��� AESŰ�� �̴ϼ� ���� ������ �ε�
	//MyName = LoadEncryptionFile();
	LoadEncryptionFile();
	TextBox.append("* Load Encryption Materials\n", false);
	MyName = "Client1";
	TextBox.append("* Your Client Name : "+ MyName + '\n', false);

	//���α׷� ����� FileNameTable �ε�
	if (std::experimental::filesystem::exists("KEY\\FileNameTable.dat")) {
		std::ifstream ifs("KEY\\FileNameTable.dat");
		boost::archive::text_iarchive ia(ifs);
		ia >> FileNameTable;
		TextBox.append("* FileNameTable Load Success\n", false);
	}


	fm.show();
	exec();


	return 0;

}
