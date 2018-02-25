#include "NoTraceModuleInjector.h"
#include <include/FileLib/File.h>

#pragma comment(lib,"FileLib.lib")

libTools::CNoTraceModuleInjector::CNoTraceModuleInjector() : _pvFileContent(nullptr), _uFileSize(0), _IsAllocMemory(FALSE)
{

}

libTools::CNoTraceModuleInjector::~CNoTraceModuleInjector()
{
	if (_IsAllocMemory && _pvFileContent != nullptr)
	{
		::VirtualFree(_pvFileContent, 0, MEM_RELEASE);
		_pvFileContent = nullptr;
	}
}

BOOL libTools::CNoTraceModuleInjector::SetModuleContent(_In_ CONST std::wstring& wsDllPath)
{
	if (!CFile::ReadFileContent(wsDllPath, _pvFileContent, _uFileSize))
	{
		return FALSE;
	}


	_IsAllocMemory = TRUE;
	return TRUE;
}

BOOL libTools::CNoTraceModuleInjector::MyLoadLibrary()
{
	return TRUE;
}

VOID libTools::CNoTraceModuleInjector::SetModuleContent(_In_ LPVOID pvFileContent, _In_ SIZE_T uSize)
{
	_pvFileContent = pvFileContent;
	_uFileSize = uSize;
	_IsAllocMemory = FALSE;
}
