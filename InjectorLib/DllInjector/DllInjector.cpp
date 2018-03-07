#include "DllInjector.h"
#include <Shlwapi.h>
#include <include/ProcessLib/Common/ResHandleManager.h>
#include <include/LogLib/Log.h>

#pragma comment(lib,"ProcessLib.lib")
#pragma comment(lib,"LogLib.lib")

#define _SELF L"DllInjector.cpp"
BOOL libTools::CDllInjector::RemoteThreadInjectorDLL(_In_ DWORD dwPid, _In_ CONST std::wstring& wsDllPath)
{
	// RaisePrivilige
	RaisePrivilige(SE_DEBUG_NAME);
	RaisePrivilige(SE_SECURITY_NAME);


	// Debug Privilige
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (hProcess == NULL)
	{
		LOG_C_E(L"OpenProcess = NULL, Error=%d", ::GetLastError());
		return FALSE;
	}
	
	// Alloc in Target Process
	SIZE_T dwDLLSize = (wsDllPath.length() + 1) * 2;
	LPVOID pAllocMem = VirtualAllocEx(hProcess, NULL, dwDLLSize, MEM_COMMIT, PAGE_READWRITE);
	if (pAllocMem == nullptr)
	{
		LOG_C_E(L"VirtualAllocEx = NULL, Error=%d", ::GetLastError());
		::CloseHandle(hProcess);
		return FALSE;
	}
	SetResDeleter(pAllocMem, [=](LPVOID& p) { ::VirtualFreeEx(hProcess, p, 0, MEM_RELEASE); p = nullptr; });
	SetResDeleter(hProcess, [](HANDLE& hProcess) { ::CloseHandle(hProcess); hProcess = NULL; });


	//将DLL的路径名复制到远程进程的地址空间
	if(!WriteProcessMemory(hProcess, pAllocMem, reinterpret_cast<LPCVOID>(wsDllPath.c_str()), dwDLLSize, NULL))
	{
		LOG_C_E(L"WriteProcessMemory = FALSE, Error=%d", ::GetLastError());
		return FALSE;
	}


	//获得LoadLibraryA在Kernel.dll中得真正地址
	// should get module addr in snap process 
	HMODULE hmKernel32 = ::GetModuleHandleW(L"kernel32.dll");
	if (hmKernel32 == NULL)
	{
		LOG_C_E(L"GetModuleHandleW = FALSE, Error=%d", ::GetLastError());
		return FALSE;
	}


	PTHREAD_START_ROUTINE pfnThreadRrn = reinterpret_cast<PTHREAD_START_ROUTINE>(::GetProcAddress(hmKernel32, "LoadLibraryW"));
	if (pfnThreadRrn == NULL)
	{
		LOG_C_E(L"GetProcAddress = FALSE, Error=%d", ::GetLastError());
		return FALSE;
	}


	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRrn, pAllocMem, 0, NULL);
	if (hThread == NULL)
	{
		LOG_C_E(L"CreateRemoteThread = FALSE, Error=%d", ::GetLastError());
		return FALSE;
	}
	

	//等待远程线程终止  
	WaitForSingleObject(hThread, INFINITE);
	SetResDeleter(hThread, [](HANDLE& hThread) { ::CloseHandle(hThread); hThread = NULL; });
	return TRUE;
}

BOOL libTools::CDllInjector::CreateProcess_And_InjectorDLL(_In_ CONST std::wstring& wsProcessCommand, _In_ CONST std::wstring& wsDllPath, _Out_opt_ PROCESS_INFORMATION* pProcInfo /*= nullptr*/)
{
	WCHAR wszProcDirectory[MAX_PATH] = { 0 };
	WCHAR wszCreateProcessCommand[MAX_PATH] = { 0 };


	std::copy(wsProcessCommand.begin(), wsProcessCommand.end(), wszCreateProcessCommand);
	std::copy(wsProcessCommand.begin(), wsProcessCommand.end(), wszProcDirectory);
	if (!PathRemoveFileSpecW(wszProcDirectory))
	{
		return FALSE;
	}


	STARTUPINFOW		si = { sizeof(STARTUPINFOW) };
	PROCESS_INFORMATION pi = { 0 };
	if (!::CreateProcessW(NULL, wszCreateProcessCommand, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, wszProcDirectory, &si, &pi))
	{
		return FALSE;
	}

	if (!wsDllPath.empty())
	{
		if (!RemoteThreadInjectorDLL(pi.dwProcessId, wsDllPath))
		{
			::TerminateProcess(pi.hProcess, 0);
			::WaitForSingleObject(pi.hProcess, INFINITE);
			::CloseHandle(pi.hProcess);
			return FALSE;
		}
	}


	if (pProcInfo != nullptr)
	{
		*pProcInfo = pi;
	}

	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hProcess);
	return TRUE;
}

BOOL libTools::CDllInjector::CreateProcess_And_ShellCodeInjectorDLL(_In_ CONST std::wstring& wsProcessCommand, _In_ CONST std::wstring& wsDllPath, _Out_opt_ PROCESS_INFORMATION* pProcInfo /*= nullptr*/)
{
	struct SHELL_CODE
	{
		WCHAR szPath[MAX_PATH];
		WCHAR szInstruction[0x20];
		SHELL_CODE()
		{
			ZeroMemory(szPath, sizeof(szPath));
			ZeroMemory(szInstruction, sizeof(szInstruction));
		}
	};


	WCHAR wszProcDirectory[MAX_PATH] = { 0 };
	WCHAR wszCreateProcessCommand[MAX_PATH] = { 0 };
	std::copy(wsProcessCommand.begin(), wsProcessCommand.end(), wszCreateProcessCommand);
	std::copy(wsProcessCommand.begin(), wsProcessCommand.end(), wszProcDirectory);
	if (!PathRemoveFileSpecW(wszProcDirectory))
	{
		return FALSE;
	}


	STARTUPINFOW		si = { sizeof(STARTUPINFOW) };
	PROCESS_INFORMATION pi = { 0 };
	if (!::CreateProcessW(NULL, wszCreateProcessCommand, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, wszProcDirectory, &si, &pi))
	{
		return FALSE;
	}


	// Injector DLL
	CONTEXT Context = { 0 };
	Context.ContextFlags = CONTEXT_INTEGER;
	GetThreadContext(pi.hThread, &Context);
	LPVOID Buffer = ::VirtualAllocEx(pi.hProcess, NULL, sizeof(SHELL_CODE), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (Buffer == NULL)
	{
		return FALSE;
	}


	CHAR szShellCode[] = "\x60\x68\x12\x34\x56\x78\xb8\x12\x34\x56\x78\xff\xd0\x61\xe9\x12\x34\x56\x78";
	/* szShellCode =
	pushad
	push 0x78563412
	mov eax, 0x78563412
	call eax
	popad
	jmp 0x78563412
	*/
	SHELL_CODE ShellCode;
	*(DWORD*)(szShellCode + 2) = reinterpret_cast<DWORD>(Buffer);
	*(DWORD*)(szShellCode + 7) = reinterpret_cast<DWORD>(LoadLibraryW);
	*(DWORD*)(szShellCode + 15) = Context.Eax - (DWORD)((PUCHAR)Buffer + FIELD_OFFSET(SHELL_CODE, szInstruction) + sizeof(szShellCode) - 1);
	CopyMemory(((SHELL_CODE *)&ShellCode)->szPath, wsDllPath.c_str(), wsDllPath.length());
	CopyMemory(((SHELL_CODE *)&ShellCode)->szInstruction, szShellCode, sizeof(szShellCode));


	if (!WriteProcessMemory(pi.hProcess, Buffer, &ShellCode, sizeof(SHELL_CODE), NULL))
	{
		::VirtualFreeEx(pi.hProcess, Buffer, NULL, MEM_RELEASE);
		return FALSE;
	}


	Context.Eax = (DWORD)(((SHELL_CODE *)Buffer)->szInstruction);
	SetThreadContext(pi.hThread, &Context);
	ResumeThread(pi.hThread);


	if (pProcInfo != nullptr)
	{
		*pProcInfo = pi;
	}

	return TRUE;
}

BOOL libTools::CDllInjector::RaisePrivilige(_In_ LPCWSTR pwszPrivilegeName)
{
	//提升权限，代码实例
	HANDLE hToken			= NULL;        // 令牌句柄
	TOKEN_PRIVILEGES tkp	= { 0 };       // 令牌结构指针
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken))
	{
		return FALSE;
	}


	if (!::LookupPrivilegeValueW(NULL, pwszPrivilegeName, &tkp.Privileges[0].Luid))
	{
		return FALSE;
	}


	tkp.PrivilegeCount = 1;  // one privilege to set   
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!::AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
	{
		return FALSE;
	}

	::CloseHandle(hToken);
	hToken = NULL;
	return TRUE;
}
