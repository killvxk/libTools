#include "Character.h"
#include <io.h>		// _setmode & _fileno
#include <fcntl.h>	// _O_U8TEXT

#pragma comment(lib,"ExceptionLib.lib")
namespace libTools
{
	std::string CCharacter::UnicodeToASCII(_In_ CONST std::wstring& wstr)
	{
		std::string str;
		CException::InvokeAction(__FUNCTIONW__, [&]
		{
			int ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
			char* szAnsi = new char[ansiLen + 1];
			ZeroMemory(szAnsi, ansiLen + 1);


			::WideCharToMultiByte(CP_ACP, NULL, wstr.c_str(), wstr.length(), szAnsi, ansiLen, NULL, NULL);
			szAnsi[ansiLen] = '\0';

			str = szAnsi;
			delete[] szAnsi;
		});

		return str;
	}

	std::wstring CCharacter::ASCIIToUnicode(_In_ CONST std::string& pszText)
	{
		std::wstring str;
		CException::InvokeAction(__FUNCTIONW__, [&]
		{
			int cchWideChar = MultiByteToWideChar(CP_ACP, 0, pszText.c_str(), -1, NULL, 0);

			WCHAR* ptszText = new wchar_t[cchWideChar];
			ZeroMemory(ptszText, cchWideChar);

			MultiByteToWideChar(CP_ACP, 0, pszText.c_str(), -1, ptszText, cchWideChar);

			ptszText[cchWideChar] = '\0';
			str = ptszText;
			delete[] ptszText;
		});

		return str;
	}

	std::wstring CCharacter::UTF8ToUnicode(_In_ CONST std::string pszText)
	{
		std::wstring str;
		CException::InvokeAction(__FUNCTIONW__, [&]
		{
			int cchWideChar = MultiByteToWideChar(CP_UTF8, 0, pszText.c_str(), -1, NULL, 0);

			WCHAR* ptszText = new wchar_t[cchWideChar];
			ZeroMemory(ptszText, cchWideChar);

			MultiByteToWideChar(CP_ACP, 0, pszText.c_str(), -1, ptszText, cchWideChar);

			ptszText[cchWideChar] = '\0';
			str = ptszText;
			delete[] ptszText;
		});

		return str;
	}

	std::string CCharacter::UnicodeToUTF8(_In_ CONST std::wstring& wsText)
	{
		std::string str;
		CException::InvokeAction(__FUNCTIONW__, [&]
		{
			DWORD dwLen = ::WideCharToMultiByte(CP_UTF8, NULL, wsText.c_str(), static_cast<int>(wsText.length()), NULL, NULL, NULL, NULL);
			char* pszUTF8 = new char[dwLen + 1];
			ZeroMemory(pszUTF8, dwLen + 1);


			::WideCharToMultiByte(CP_UTF8, NULL, wsText.c_str(), static_cast<int>(wsText.length()), pszUTF8, dwLen, NULL, NULL);
			pszUTF8[dwLen] = '\0';

			str = pszUTF8;
			delete[] pszUTF8;
		});

		return str;
	}

	std::wstring CCharacter::MakeFormatText(_In_ LPCWSTR pwszFormat, ...)
	{
		va_list		args;

		va_start(args, pwszFormat);
		size_t BufferSize = vswprintf(nullptr, 0, pwszFormat, args);
		if (BufferSize < 1024 - 2/*\0*/)
		{
			wchar_t		szBuff[1024] = { 0 };
			_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
			va_end(args);

			std::wstring wsFormatText = szBuff;
			return wsFormatText;
		}

		WCHAR* Buffer = new WCHAR[BufferSize + 2];
		ZeroMemory(Buffer, BufferSize + 2);

		_vsnwprintf_s(Buffer, BufferSize + 1, _TRUNCATE, pwszFormat, args);
		va_end(args);

		std::wstring wsFormatText = Buffer;
		delete[] Buffer;
		return wsFormatText;
	}

	VOID CCharacter::GetVecByParm_RemoveQuotes(_In_ CONST std::wstring& wsText, _In_ WCHAR wchKeyword, _Out_opt_ std::vector<std::wstring>& VecParm)
	{
		std::wstring wsParm;
		BOOL bQuotes = FALSE;
		for (auto& itm : wsText)
		{
			if (!bQuotes && itm == wchKeyword)
			{
				VecParm.push_back(wsParm);
				wsParm.clear();
				continue;
			}

			if (itm == '\"')
			{
				bQuotes = !bQuotes;
				continue;
			}

			wsParm.push_back(itm);
		}

		if (!wsParm.empty())
			VecParm.push_back(std::move(wsParm));

		auto uParamCount = GetCount_By_SpecifyText(wsText, std::wstring(L",")) + 1;
		while (VecParm.size() < uParamCount)
		{
			VecParm.push_back(L"");
		}
	}

	BOOL CCharacter::SplitFormatText(_In_ CONST std::wstring& wsText, _In_ CONST std::wstring& wsFormatText, _Out_opt_ std::vector<std::wstring>& VecResult)
	{
		std::wstring wsVarFormatText = wsFormatText;
		std::wstring wsVarText = wsText;

		auto VecFormatText = FindSplitFormatText(wsVarFormatText);
		for (CONST auto& itm : VecFormatText)
		{
			std::wstring wsResult;
			GetRemoveLeft(wsVarText, itm, wsResult);


			if (!wsResult.empty())
			{
				VecResult.push_back(wsResult);
			}


			GetRemoveRight(wsVarText, itm, wsVarText);
		}


		if (!wsVarText.empty())
		{
			VecResult.push_back(wsVarText);
		}
		return TRUE;
	}

	std::wstring CCharacter::MakeCurrentPath(_In_ CONST std::wstring& wsText)
	{
		WCHAR wszPath[MAX_PATH] = { 0 };
		::GetCurrentDirectoryW(MAX_PATH, wszPath);

		return std::wstring(wszPath) + wsText;
	}

	VOID CCharacter::SetConsoleLanguage()
	{
		setlocale(LC_ALL, "");
	}

	VOID CCharacter::SetSpecialCharacterMode()
	{
		_setmode(_fileno(stdout), _O_U8TEXT);
	}

	std::vector<std::wstring> CCharacter::FindSplitFormatText(_In_ CONST std::wstring& wsFormatText)
	{
		std::vector<std::wstring> Vec;

		std::wstring wsText;
		for (auto itr = wsFormatText.begin(); itr != wsFormatText.end(); ++itr)
		{
			if (*itr != L'*')
			{
				wsText += *itr;
				continue;
			}

			if (!wsText.empty())
			{
				Vec.push_back(wsText);
				wsText.clear();
			}
		}

		if (!wsText.empty())
		{
			Vec.push_back(wsText);
		}

		return Vec;
	}
}