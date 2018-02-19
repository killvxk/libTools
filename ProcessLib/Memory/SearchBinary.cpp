#include "SearchBinary.h"
#include "Memory.h"

DWORD libTools::CSearchBinary::FindAddr(LPCSTR lpszCode, int nOffset, int nOrderNum, LPCWSTR lpszModule)
{
	DWORD	dwArray[10] = { 0 };
	UINT	uArrayLen = 0x0;
	DWORD	dwAddr = 0x0;

	//开始搜索基址
	if (SearchBase(lpszCode, dwArray, uArrayLen, lpszModule))
	{
		if (uArrayLen == 1)//判断只有一个的时候
			dwAddr = dwArray[0];
		else
			dwAddr = dwArray[nOrderNum];
	}

	if (dwAddr != 0x0) {
		if (nOffset >= 0)
			dwAddr -= abs(nOffset);
		else
			dwAddr += abs(nOffset);
	}


	return dwAddr;
}

DWORD libTools::CSearchBinary::FindCALL(LPCSTR lpszCode, int nOffset, DWORD dwModuleAddr, int nMov, int nOrderNum, LPCWSTR lpszModule)
{
	DWORD	dwArray[10] = { 0 };
	UINT	uArrayLen = 0x0;
	DWORD	dwCALL = 0x0;
	DWORD	dwAddr = 0x0;

	//开始搜索基址
	if (SearchBase(lpszCode, dwArray, uArrayLen, lpszModule))
	{
		if (uArrayLen == 1)//判断只有一个的时候
			dwAddr = dwArray[0];
		else
			dwAddr = dwArray[nOrderNum];
	}

	if (dwAddr != 0x0)
	{
		if (nOffset >= 0)
			dwAddr -= abs(nOffset);
		else
			dwAddr += abs(nOffset);

		// 不是CALL!
		if (CMemory::ReadBYTE(dwAddr) != 0xE8)
			return 0x0;

		//首先计算相对地址
		DWORD dwRelativeAddr = dwAddr - (dwModuleAddr + 0x1000) + 0x1000 + nMov;
		dwRelativeAddr += dwModuleAddr;
		DWORD dwReadAddr = CMemory::ReadDWORD(dwRelativeAddr);
		dwReadAddr += 4;
		dwReadAddr += dwRelativeAddr;
		dwCALL = dwReadAddr & 0xFFFFFFFF;
	}

	return dwCALL;
}

DWORD libTools::CSearchBinary::FindBase(LPCSTR lpszCode, int nOffset, int nMov, int nOrderNum, LPCWSTR lpszModule, DWORD dwAddrLen /*= 0xFFFFFFFF*/)
{
	DWORD	dwArray[10] = { 0 };
	UINT	uArrayLen = 0x0;
	DWORD	dwBase = 0x0;
	DWORD	dwAddr = 0x0;

	//开始搜索基址
	if (SearchBase(lpszCode, dwArray, uArrayLen, lpszModule))
	{
		if (uArrayLen == 1)//判断只有一个的时候
			dwAddr = dwArray[0];
		else
			dwAddr = dwArray[nOrderNum];
	}

	if (dwAddr != 0x0)
	{
		if (nOffset >= 0)
			dwAddr -= abs(nOffset);
		else
			dwAddr += abs(nOffset);

		dwAddr += nMov;
		dwBase = CMemory::ReadDWORD(dwAddr)&dwAddrLen;
	}

	return dwBase;
}

DWORD libTools::CSearchBinary::FindBase_ByCALL(LPCSTR lpszCode, int nOffset, DWORD dwModuleAddr, int nMov, int nOrderNum, LPCWSTR lpszModule, int nBaseOffset, DWORD dwAddrLen /*= 0xFFFFFFFF*/)
{
	DWORD dwCALL = FindCALL(lpszCode, nOffset, dwModuleAddr, nMov, nOrderNum, lpszModule);
	if (dwCALL == NULL)
		return NULL;

	dwCALL += nBaseOffset;
	return CMemory::ReadDWORD(dwCALL) & dwAddrLen;
}

BOOL libTools::CSearchBinary::SearchBase(LPCSTR szCode, DWORD * pArray, UINT& puLen, LPCWSTR lpszModule)
{
	SYSTEM_INFO		si;
	MEMORY_BASIC_INFORMATION		mbi;

	BOOL	bRetCode = FALSE;

	//将字符串转换成BYTE数组
	UINT uCodeLen = static_cast<UINT>(strlen(szCode)) / 2;
	if (strlen(szCode) % 2 != 0)
	{
		MessageBoxW(NULL, L"必须是2的倍数!", L"Error", NULL);
		return FALSE;
	}

	DWORD * pCode = new DWORD[uCodeLen];
	memset(pCode, 0, uCodeLen);

	for (UINT i = 0; i < uCodeLen; ++i)
	{
		if (szCode[i * 2] != '?')
		{
			char szText[] = { szCode[i * 2], szCode[i * 2 + 1], '\0' };
			pCode[i] = (DWORD)strtol(szText, NULL, 16);
		}
		else
		{
			pCode[i] = 0x100;
		}
	}

	//初始化
	::GetSystemInfo(&si);
	HANDLE hProcess = ::GetCurrentProcess();
	DWORD dwImageBase = static_cast<DWORD>(reinterpret_cast<UINT_PTR>(::GetModuleHandleW(lpszModule)));
	if (dwImageBase == NULL)
	{
		return FALSE;
	}

	DWORD dwImageSize = GetImageSize((HMODULE)dwImageBase);
	if (dwImageSize == NULL)
	{
		return FALSE;
	}

	for (DWORD dwAddr = dwImageBase; dwAddr < dwImageBase + dwImageSize; dwAddr += mbi.RegionSize)
		//for (BYTE * pCurPos = (LPBYTE)si.lpMinimumApplicationAddress; pCurPos < (LPBYTE)si.lpMaximumApplicationAddress - uCodeLen; pCurPos = (PBYTE)mbi.BaseAddress + mbi.RegionSize,nCount++ )
	{
		//查询当前内存页的属性
		::VirtualQueryEx(hProcess, (LPCVOID)dwAddr, &mbi, sizeof(mbi));
		if (mbi.Protect == PAGE_READONLY)//扫描只读内存
		{
			DWORD dwOldProtect;
			::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dwOldProtect);
		}
		if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_EXECUTE_READ || mbi.Protect == PAGE_EXECUTE_READWRITE))
		{
			std::vector<int> vlst;
			CL_sunday(pCode, uCodeLen, (PBYTE)mbi.BaseAddress, mbi.RegionSize, vlst);

			for (UINT i = 0; i < vlst.size() && puLen < 10; ++i)
			{
				pArray[puLen] = (DWORD)mbi.BaseAddress + vlst.at(i);
				++puLen;
			}

		}
	}

	if (puLen >= 10)
	{
		bRetCode = TRUE;
	}
	else if (puLen < 10 && puLen > 0)
	{
		bRetCode = TRUE;
	}
	delete[] pCode;
	return bRetCode;
}

DWORD libTools::CSearchBinary::GetImageSize(HMODULE hm)
{
	DWORD dwSize = 0x0;
	_asm
	{
		PUSHAD
		MOV EBX, DWORD PTR hm
		MOV EAX, DWORD PTR DS : [EBX + 0x3C]
		LEA EAX, DWORD PTR DS : [EBX + EAX + 0x50]
		MOV EAX, DWORD PTR DS : [EAX]
		MOV DWORD PTR DS : [dwSize], EAX
		POPAD
	}
	return dwSize;
}

BOOL libTools::CSearchBinary::CL_sunday(DWORD* pKey, UINT uKeyLen, BYTE* pCode, UINT uCodeLen, std::vector<int>& vlst)
{
	//807E1000740E
	UINT uNowPos = 0;
	while (uNowPos + uKeyLen < uCodeLen)
	{
		if (CompCode(pKey, pCode + uNowPos, uKeyLen))
		{
			vlst.push_back(uNowPos);
			uNowPos += uKeyLen + 1;
			continue;
		}

		BYTE bWord = pCode[uKeyLen + uNowPos];

		int nWordPos = GetWord_By_Char(bWord, pKey, uKeyLen);
		if (nWordPos == -1)
			uNowPos += uKeyLen + 1;
		else
			uNowPos += (UINT)nWordPos;
	}

	return TRUE;
}

int libTools::CSearchBinary::GetWord_By_Char(BYTE dwWord, DWORD* pKey, UINT uKeyLen)
{
	int uLen = uKeyLen - 1;
	for (int i = uLen; i >= 0; --i)
	{
		if (pKey[i] == 0x100 || (BYTE)pKey[i] == dwWord)
		{
			return uKeyLen - i;
		}
	}
	return -1;
}

BOOL libTools::CSearchBinary::CompCode(const DWORD * pCode, const BYTE * pMem, UINT uLen)
{
	BOOL bComp = TRUE;
	for (UINT i = 0; i < uLen; ++i)
	{
		if (pCode[i] != 0x100 && (BYTE)pCode[i] != (BYTE)pMem[i])
		{
			bComp = FALSE;
			break;
		}
	}

	return bComp;
}

