#include "File.h"
#include <memory>
#include <Shlwapi.h> // PathFileExistsW
#include <include/CharacterLib/Character.h>
#include <include/ExceptionLib/Exception.h>


#pragma comment(lib,"CharacterLib.lib")
#pragma comment(lib,"ExceptionLib.lib")
#pragma comment(lib,"Shlwapi.lib")

namespace libTools
{
	BOOL CFile::CreateMyDirectory(IN LPCWSTR pwchText, BOOL bShowErr /* = FALSE */)
	{
		SECURITY_ATTRIBUTES Atribute;
		Atribute.bInheritHandle = FALSE;
		Atribute.lpSecurityDescriptor = NULL;
		Atribute.nLength = sizeof(SECURITY_ATTRIBUTES);

		if (!::CreateDirectoryW(pwchText, &Atribute) && bShowErr) {
			MessageBoxW(NULL, L"创建文件夹失败!这不科学啊~", L"错误", NULL);
			return FALSE;
		}
		return TRUE;
	}

	BOOL CFile::ReadUnicodeFile(_In_ CONST std::wstring& wsPath, _Out_ std::wstring& wsContent)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			FILE* pFile = nullptr;
			_wfopen_s(&pFile, wsPath.c_str(), L"rb");
			if (pFile == nullptr)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"ReadUnicodeFile Fiald! Path:%s", wsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			fseek(pFile, 0, SEEK_END);
			LONG lLen = (ftell(pFile) + 2) / 2;


			std::shared_ptr<WCHAR> pwstrBuffer(new WCHAR[lLen], [](WCHAR* p) {delete[] p; });

			ZeroMemory(pwstrBuffer.get(), lLen * sizeof(WCHAR));
			fseek(pFile, 0, SEEK_SET);
			fread(pwstrBuffer.get(), sizeof(WCHAR), (size_t)lLen - 1, pFile);
			pwstrBuffer.get()[lLen - 1] = '\0';

			wsContent = pwstrBuffer.get() + ((pwstrBuffer.get()[0] == 0xFEFF) ? 1 : 0);
			fclose(pFile);
			return TRUE;
		});
	}


	BOOL WINAPI CFile::ReadASCIIFile(_In_ CONST std::wstring& cwsPath, _Out_ std::wstring& cwsContent)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			FILE* pFile = nullptr;
			_wfopen_s(&pFile, cwsPath.c_str(), L"r");
			if (pFile == nullptr)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"ReadASCIIFile Fiald! Path:%s", cwsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			fseek(pFile, 0, SEEK_END);
			LONG lLen = ftell(pFile) + 1;

			std::shared_ptr<CHAR> pwstrBuffer(new CHAR[lLen], [](CHAR* p) {delete[] p; });

			ZeroMemory(pwstrBuffer.get(), lLen * sizeof(CHAR));
			fseek(pFile, 0, SEEK_SET);
			fread(pwstrBuffer.get(), sizeof(CHAR), (size_t)lLen - 1, pFile);
			pwstrBuffer.get()[lLen - 1] = '\0';

			cwsContent = CCharacter::ASCIIToUnicode(pwstrBuffer.get());
			fclose(pFile);
			return TRUE;
		});
	}

	BOOL CFile::WriteUnicodeFile(_In_ CONST std::wstring& wsPath, _In_ CONST std::wstring& wsContent)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			// Exist Write File Mutex
			if (!FileExist(wsPath.c_str()) && !CreateUnicodeTextFile(wsPath))
				return FALSE;

			FILE* pFile = nullptr;
			_wfopen_s(&pFile, wsPath.c_str(), L"wb+");
			if (pFile == nullptr)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"WriteUnicodeFile Fiald! Path:%s", wsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			std::shared_ptr<WCHAR> pwstrBuffer(new WCHAR[wsContent.length() + 1], [](WCHAR* p) {delete[] p; });
			pwstrBuffer.get()[0] = 0xFEFF;
			memcpy(pwstrBuffer.get() + 1, wsContent.c_str(), wsContent.length() * 2);

			fseek(pFile, 0, SEEK_SET);

			fwrite(pwstrBuffer.get(), sizeof(WCHAR), wsContent.length() + 1, pFile);
			fclose(pFile);
			return TRUE;
		});
	}

	BOOL WINAPI CFile::WriteASCIIFile(_In_ CONST std::wstring& wsPath, _In_ CONST std::wstring& wsContent)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			// Exist Write File Mutex
			if (!FileExist(wsPath.c_str()) && !CreateASCIITextFile(wsPath))
				return FALSE;

			FILE* pFile = nullptr;
			_wfopen_s(&pFile, wsPath.c_str(), L"a+");
			if (pFile == nullptr)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"WriteASCIIFile Fiald! Path:%s", wsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			std::string str = CCharacter::UnicodeToASCII(wsContent);

			fseek(pFile, 0, SEEK_SET);
			fwrite(str.c_str(), sizeof(CHAR), str.length(), pFile);
			fclose(pFile);
			return TRUE;
		});
	}

	BOOL WINAPI CFile::WriteFile(_In_ CONST std::wstring& cwsPath, _In_ CONST BYTE* Buffer, _In_ UINT uSize)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			FILE* pFile = nullptr;
			_wfopen_s(&pFile, cwsPath.c_str(), L"wb+");
			if (pFile == nullptr)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"WriteFile Fiald! Path:%s", cwsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			fseek(pFile, 0, SEEK_SET);

			fwrite(Buffer, sizeof(BYTE), uSize, pFile);
			fclose(pFile);
			pFile = nullptr;
			return TRUE;
		});
	}

	BOOL WINAPI CFile::AppendFile(_In_ CONST std::wstring& cwsPath, _In_ CONST BYTE* Buffer, _In_ UINT uSize)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			FILE* pFile = nullptr;
			_wfopen_s(&pFile, cwsPath.c_str(), L"ab+");
			if (pFile == nullptr)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"AppendFile Fiald! Path:%s", cwsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			fseek(pFile, 0, SEEK_SET);

			fwrite(Buffer, sizeof(BYTE), uSize, pFile);
			fclose(pFile);
			pFile = nullptr;
			return TRUE;
		});
	}

	BOOL WINAPI CFile::AppendUnicodeFile(_In_ CONST std::wstring& wsPath, _In_ CONST std::wstring& cwsContent)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			// Exist Write File Mutex
			if (!FileExist(wsPath.c_str()) && !CreateUnicodeTextFile(wsPath))
				return FALSE;

			FILE* pFile = nullptr;
			_wfopen_s(&pFile, wsPath.c_str(), L"ab+");
			if (pFile == nullptr)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"AppendUnicodeFile Fiald! Path:%s", wsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			fseek(pFile, 0, SEEK_END);

			std::wstring wsContent = cwsContent;
			if (cwsContent[cwsContent.length() - 1] != '\n')
				wsContent.append(L"\r\n");

			fwrite(wsContent.c_str(), sizeof(WCHAR), wsContent.length(), pFile);
			fclose(pFile);
			return TRUE;
		});
	}

	BOOL WINAPI CFile::CreateUnicodeTextFile(_In_ CONST std::wstring& cwsPath)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			FILE* pFile = nullptr;
			_wfopen_s(&pFile, cwsPath.c_str(), L"a");
			if (pFile == NULL)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"创建文件:%s 失败!", cwsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			fseek(pFile, 0, SEEK_SET);
			WCHAR wszFlag = 0xFEFF;
			fwrite(&wszFlag, sizeof(WCHAR), 1, pFile);
			fclose(pFile);
			return TRUE;
		});
	}

	BOOL WINAPI CFile::CreateASCIITextFile(_In_ CONST std::wstring& cwsPath)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			FILE* pFile = nullptr;
			_wfopen_s(&pFile, cwsPath.c_str(), L"a");
			if (pFile == NULL)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"创建文件:%s 失败!", cwsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			fclose(pFile);
			return TRUE;
		});
	}

	BOOL WINAPI CFile::ClearFileContent(_In_ CONST std::wstring& cwsPath)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			FILE* pFile = nullptr;
			_wfopen_s(&pFile, cwsPath.c_str(), L"w+");
			if (pFile != NULL)
				fclose(pFile);

			pFile = nullptr;
			return TRUE;
		});
	}

	BOOL WINAPI CFile::ReadFileContent(_In_ CONST std::wstring& wsPath, _Out_ LPVOID& lpFileContent, _Out_ SIZE_T& uSize)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			HANDLE hFile = ::CreateFileW(wsPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (INVALID_HANDLE_VALUE == hFile)
				return FALSE;

			uSize = ::GetFileSize(hFile, NULL);
			if (uSize == 0)
			{
				::CloseHandle(hFile);
				return FALSE;
			}

			lpFileContent = ::VirtualAlloc(NULL, uSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (lpFileContent == nullptr)
			{
				::CloseHandle(hFile);
				return FALSE;
			}

			DWORD dwReadSize = 0;
			if (!ReadFile(hFile, lpFileContent, uSize, &dwReadSize, NULL))
			{
				::VirtualFree(lpFileContent, 0, MEM_RELEASE);
				::CloseHandle(hFile);
				return FALSE;
			}
			::CloseHandle(hFile);
			return TRUE;
		});
	}



	BOOL WINAPI CFile::ReadUnicodeConfig(_In_ CONST std::wstring& wsConfigPath, _In_ CONST std::wstring& wsConfigText, _In_ CONST std::wstring& wsConfigKey, _Out_ std::wstring& wsConfigValue)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			WCHAR wszText[MAX_PATH] = { 0 };
			::GetCurrentDirectoryW(MAX_PATH, wszText);
			::lstrcatW(wszText, wsConfigPath.c_str());

			if (!FileExist(wszText))
				return FALSE;

			WCHAR wszConfigValue[MAX_PATH] = { 0 };
			::GetPrivateProfileStringW(wsConfigText.c_str(), wsConfigKey.c_str(), L"Error", wszConfigValue, MAX_PATH, wszText);
			wsConfigValue = wszConfigValue;
			return wsConfigValue != L"Error" ? TRUE : FALSE;
		});
	}

	BOOL WINAPI CFile::FileExist(_In_ CONST std::wstring& wsPath)
	{
		return PathFileExistsW(wsPath.c_str());
	}

	BOOL WINAPI CFile::ReadAsciiFileLen(_In_ CONST std::wstring& cwsPath, _Out_ ULONG& ulFileLen)
	{
		return CException::InvokeFunc<BOOL>(__FUNCTIONW__, [&]
		{
			FILE* pFile = nullptr;
			_wfopen_s(&pFile, cwsPath.c_str(), L"rb");
			if (pFile == nullptr)
			{
				::MessageBoxW(NULL, CCharacter::MakeFormatText(L"ReadScriptFile :%s 失败!", cwsPath.c_str()).c_str(), L"Error", NULL);
				return FALSE;
			}

			fseek(pFile, 0, SEEK_END);
			LONG lLen = ftell(pFile);
			fclose(pFile);

			ulFileLen = lLen;
			return TRUE;
		});
	}


	BOOL WINAPI CFile::DirectoryExist(_In_ CONST std::wstring& wsPath)
	{
		DWORD dwType = GetFileAttributesW(wsPath.c_str());
		if (dwType == INVALID_FILE_ATTRIBUTES)
			return FALSE;

		return dwType & FILE_ATTRIBUTE_DIRECTORY ? TRUE : FALSE;
	}
}