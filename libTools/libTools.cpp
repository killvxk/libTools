// libTools.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <tlhelp32.h>
#include <AlgroithmLib/Encrypt/RSA.h>
#include <sstream>
#include <iterator>


#pragma comment(lib,"AlgroithmLib.lib")
#define _SELF L"asd"
int main()
{
	static libTools::CRSA Rsa;
	CONST static libTools::CRSA::PublicKeyClass PubKey = { 0x101, 0x679051 };
	//CRSA::PrivateKeyClass PriKey;
	//r.GenerateKey(&PubKey, &PriKey);

	//std::wcout << std::hex << PubKey.exponent << L"," << PubKey.modulus << std::endl;
	//std::wcout << std::hex << PriKey.exponent << L"," << PriKey.modulus << std::endl;

	std::wstring wsText = L"admin,2018-3-19 14:40:33";
	std::vector<LONGLONG> VecBinary;
	Rsa.Encrypt(PubKey, wsText, VecBinary);


	wsText.clear();
	wsText.reserve(wsText.length());

	std::wstringstream Result;
	std::copy(VecBinary.begin(), VecBinary.end(), std::ostream_iterator<LONGLONG, WCHAR>(Result, L","));
	std::wcout << Result.str().c_str() << std::endl;
	//std::vector<LONGLONG> VecEncrypt;
	//r.Encrypt(PubKey, L"2018年3月19日 13:15:34", VecEncrypt);
	system("pause");
	//std::wstring wsText;
	//r.Decrypt(PriKey, VecEncrypt, wsText);
    return 0;
}

