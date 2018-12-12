#pragma once
#include <string>
#include <filesystem>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/aes.h>
#include <crypto++/modes.h>
using namespace CryptoPP;

#include "base64.hpp"

SecByteBlock key;
SecByteBlock iv;


void GenAndStoreAESKeyAndIV()
{
	AutoSeededRandomPool rnd;


	std::experimental::filesystem::create_directories(".//KEY");

	// Generate a random key
	SecByteBlock key(0x00, AES::DEFAULT_KEYLENGTH);
	rnd.GenerateBlock(key, key.size());

	FileSink SinkKey(".\\KEY\\AESkey.dat");
	SinkKey.Put(key.begin(), key.size());

	// Generate a random IV
	SecByteBlock iv(AES::BLOCKSIZE);
	rnd.GenerateBlock(iv, iv.size());

	FileSink SinkIV(".\\KEY\\AESIV.dat");
	SinkIV.Put(iv.begin(), iv.size());
}

bool IsKeyAndIVFileExist()
{
	return (std::experimental::filesystem::exists(".\\KEY\\AESkey.dat") &&
		std::experimental::filesystem::exists(".\\KEY\\AESIV.dat"));
}


void LoadKeyAndIV()
{
	string AESKeyString;
	string AESIVString;
	FileSource AESKeySink(".\\KEY\\AESkey.dat", true, new StringSink(AESKeyString));
	FileSource AESIV(".\\KEY\\AESIV.dat", true, new StringSink(AESIVString));


	byte key1[CryptoPP::AES::DEFAULT_KEYLENGTH];
	memcpy(key1, AESKeyString.c_str(), AESKeyString.length());
	byte iv1[CryptoPP::AES::DEFAULT_KEYLENGTH];
	memcpy(iv1, AESIVString.c_str(), AESIVString.length());

	key.Assign(key1, CryptoPP::AES::DEFAULT_KEYLENGTH);
	iv.Assign(iv1, CryptoPP::AES::DEFAULT_KEYLENGTH);
}

void AESEncryptionFunction(byte PlainText[], byte ChiperText[], size_t messageLen)
{
	CFB_Mode<AES>::Encryption cfbEncryption(key, key.size(), iv);
	cfbEncryption.ProcessData(PlainText, ChiperText, messageLen);
}

void AESDecryptionFunction(byte PlainText[], byte ChiperText[], size_t messageLen)
{
	CFB_Mode<AES>::Decryption cfbDecryption(key, key.size(), iv);
	cfbDecryption.ProcessData(PlainText, ChiperText, messageLen);
}


string AESEncrypteFileToBase64(string path)
{

	if (key.empty() || iv.empty() ||
		!std::experimental::filesystem::exists(path))
	{
		cout << "key or iv is not loaded or file path don't exist" << endl;
		return string("");
	}


	std::ifstream infile(path, std::ios::binary);

	infile.seekg(0, infile.end);
	size_t length = infile.tellg();
	infile.seekg(0, infile.beg);

	char *pBuffer = new char[length + 1];
	memset(pBuffer, 0, length + 1);
	infile.read(pBuffer, length);


	AESEncryptionFunction((byte *)pBuffer, (byte *)pBuffer, length);



	string Output = base64_encode(reinterpret_cast<const unsigned char*>(pBuffer), length);
	



	infile.close();
	delete pBuffer;
	return Output;
}

string DecodeBase64String(string Base64)
{
	string Output;
	Output = base64_decode(Base64);
	return Output;
}

std::vector<char> AESDecrypteDecodeBase64(string AESBase64string)
{
	string AESEncrpt = DecodeBase64String(AESBase64string);


	std::vector<char> Buffer(AESEncrpt.length() + 1);
	Buffer[AESEncrpt.length()] ='\0';


	AESDecryptionFunction((byte *)&Buffer[0], (byte *)AESEncrpt.c_str(), AESEncrpt.length());


	return Buffer;
}


