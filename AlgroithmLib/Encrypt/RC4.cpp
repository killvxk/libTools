#include "RC4.h"
#include <algorithm>

libTools::CRC4::CRC4(_In_ CONST std::string& pszKey)
{
	_uKeyLen = pszKey.length();

	K.reserve(_uKeyLen);
	for (UINT i = 0; i < _uKeyLen; ++i)
		K.push_back(pszKey[i]);
}

void libTools::CRC4::GetKeyStream(_In_ UINT uEnctypTextLength, _Out_ std::string& puszKeyStream)
{
	keyStream(uEnctypTextLength);

	puszKeyStream.reserve(uEnctypTextLength + 1);
	for (CONST auto& itm : k)
	{
		puszKeyStream.push_back(itm);
	}
}

void libTools::CRC4::GetEncryptText(_In_ CONST std::string& strPlainText, _Out_ std::string& pszEnrypText)
{
	UINT uTextLength = strPlainText.length();
	keyStream(uTextLength);

	pszEnrypText.reserve(uTextLength);
	for (UINT i = 0; i < uTextLength; ++i)
	{
		pszEnrypText.push_back(strPlainText.at(i) ^ k.at(i));
	}
}

void libTools::CRC4::Initial()
{
	for (int i = 0; i < 256; ++i)
	{
		S[i] = static_cast<UCHAR>(i);
		T[i] = K[i % _uKeyLen];
	}
}

void libTools::CRC4::rangeS()
{
	int j = 0;
	for (int i = 0; i < 256; ++i)
	{
		j = (j + S[i] + T[i]) % 256;
		S[i] = S[i] + S[j];
		S[j] = S[i] - S[j];
		S[i] = S[i] - S[j];
	}
}

void libTools::CRC4::keyStream(_In_ UINT uLen)
{
	Initial();
	rangeS();

	int m = 0, j = 0, t = 0;
	for(UINT i = 0;i < uLen; ++i)
	{
		m = (m + 1) % 256;
		j = (j + S[m]) % 256;

		S[m] = S[m] + S[j];
		S[j] = S[m] - S[j];
		S[m] = S[m] - S[j];

		t = (S[m] + S[j]) % 256;
		k.push_back(S[t]);
	}
}

void libTools::CRC4_Decryption::DecryptText(_In_ CONST std::string& pszEnctryText, _In_ CONST std::string& pszKey, _Out_ std::string& pszDecrypText)
{
	UINT uEncryTextSize = pszEnctryText.length();
	pszDecrypText.reserve(uEncryTextSize + 1);
	for (UINT i = 0;i < uEncryTextSize; ++i)
	{
		pszDecrypText.push_back(pszEnctryText.at(i) ^ pszKey.at(i));
	}
	pszDecrypText.push_back('\0');
}
