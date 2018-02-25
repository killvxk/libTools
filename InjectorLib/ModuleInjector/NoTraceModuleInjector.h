#ifndef __LIBTOOLS_INJECTORLIB_MODULEINJECTOR_NOTRACEMODULEINJECTOR_H__
#define __LIBTOOLS_INJECTORLIB_MODULEINJECTOR_NOTRACEMODULEINJECTOR_H__

#include <Windows.h>
#include <string>

namespace libTools
{
	class CNoTraceModuleInjector
	{
	public:
		CNoTraceModuleInjector();
		~CNoTraceModuleInjector();

		BOOL SetModuleContent(_In_ CONST std::wstring& wsDllPath);
		VOID SetModuleContent(_In_ LPVOID pvFileContent, _In_ SIZE_T uSize);

		// warning... DLL can not use GetModuleHandle.... or other alied API
		BOOL MyLoadLibrary();

	private:
		LPVOID _pvFileContent;
		SIZE_T _uFileSize;
		BOOL   _IsAllocMemory;
	};
}

#endif // !__LIBTOOLS_INJECTORLIB_MODULEINJECTOR_NOTRACEMODULEINJECTOR_H__
