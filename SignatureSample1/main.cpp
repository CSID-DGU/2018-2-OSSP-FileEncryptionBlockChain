
//키를 생성하고 입력된 문자열로 전자서명을 생성한후 서버에 인증요청을 하는 모듈
// GenKeyPairAndSend 부분은 키를 생성하고 개인키와 공개키를 



#include <string>
using namespace std;

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>

using namespace CryptoPP;

#include "json.hh"
using json = nlohmann::json;


#include "client_http.hpp"
#include "server_http.hpp"
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

InvertibleRSAFunction privkey;
RSAFunction pubkey;



void GenKeyPair(void)
{
	//Generate private key and public key
	AutoSeededRandomPool rng;
	privkey.Initialize(rng, 512);
	RSAFunction _pubkey(privkey);
	pubkey = _pubkey;
}




// Port : Verification Server Port Number
// SignatureMessage : String to be Verification
// Encode key and generate signature , and POST to Server address that passed as first argument
void SignMessageAndVerifPOST(string address, string SignatureMessage)
{


	AutoSeededRandomPool rng;
	string Signature;
	string Publickey;


	//Encoding Publickey with Base64 Encoder
	Base64Encoder pubkeysink(new StringSink(Publickey));
	pubkey.DEREncode(pubkeysink);
	pubkeysink.MessageEnd();
	std::cout << "Publickey Gen > " << endl;
	std::cout << Publickey << endl;

	
	//Signing SignatureMessage with Signer
	RSASSA_PKCS1v15_SHA_Signer Signer(privkey);
	SecByteBlock sbbSignature(Signer.SignatureLength());
	Signer.SignMessage(
		rng,
		(byte const*)SignatureMessage.data(),
		SignatureMessage.size(),
		sbbSignature);

	//Encoding Signature with Base64 Encoder
	StringSource(sbbSignature, sbbSignature.size(), true, new Base64Encoder(new StringSink(Signature)));
	std::cout << "Signature Gen > " << endl;
	std::cout << Signature << endl;
	std::cout << "Sending PublicKey And Signature" << endl;

	//Serialize PublicKey and Signature to JSON
	json j;
	j["PublicKey"] = Publickey.c_str(); 
	j["Signature"] = Signature.c_str();


	//POST PublicKey and Signature to 'address'
	HttpClient client(address);
	try {
		auto req = client.request("POST", "/SignRequest", j.dump());
		std::cout << req->content.string() << endl;
	}
	catch (const SimpleWeb::system_error &e) {
		cerr << "Client request error: " << e.what() << endl;
	}

}



int main()
{
	HttpServer server;
	server.config.port = 80;
	string SignatureMessage = "A message to be signed";


	//여기 코드는 다른 컴퓨터(클라우드 노드)에서 실행되야할 코드임
	server.resource["^/SignRequest$"]["POST"] = [&SignatureMessage](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		printf("POST /SignRequest --- Verification request processing\n");
		try {
			//Parsing stirng -> JSON
			json content = json::parse(request->content);

			//Parsing JSON -> string
			string PublicKey = content["PublicKey"].get<string>();
			string Signature = content["Signature"].get<string>();


			//print Encoded publickey and signature
			std::cout << "Received Publickey Content > " << endl;
			std::cout << PublicKey << endl;
			std::cout << "Received Sugnature Content > " << endl;
			std::cout << Signature << endl;


			//Decode and Load publickey and signature 
			ByteQueue Keybytes;
			StringSource KeySource(PublicKey.c_str(), true, new Base64Decoder);
			StringSource SignSource(Signature.c_str(), true, new Base64Decoder);
			KeySource.TransferTo(Keybytes);
			Keybytes.MessageEnd();
			string SignatureDecode;
			StringSink SinkSignatureDecode(SignatureDecode);
			SignSource.TransferTo(SinkSignatureDecode);
			RSA::PublicKey pubKey;
			pubKey.Load(Keybytes);
			std::cout << " SignatureDecode Contetn > " << endl;
			std::cout << SignatureDecode << endl;



			RSASSA_PKCS1v15_SHA_Verifier verifier(pubKey);
			string combined = SignatureMessage + SignatureDecode;

			
			//Verifying signature
			try
			{
				StringSource(combined, true,
					new SignatureVerificationFilter(
						verifier, NULL,
						SignatureVerificationFilter::THROW_EXCEPTION
					)
				);
				cout << "Verification OK" << endl;
			}
			catch (SignatureVerificationFilter::SignatureVerificationFailed &err)
			{
				cout << err.what() << endl;
			}
			
			response->write("Receipt Success");
		}
		catch (const exception &e) {
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	/*
	// start server
	thread server_thread([&server]() {
		server.start();
	});
	*/

	// 지갑 프로그램 쪽의 코드.
	GenKeyPair();
	SignMessageAndVerifPOST("35.221.101.60:80", "A message to be signed");

	
	//server_thread.join();
	return 0;
}
