#ifndef __LIBTOOLS_HOOKLIB_INLINEHOOK_LDRHEADER
#define __LIBTOOLS_HOOKLIB_INLINEHOOK_LDRHEADER

#pragma warning(disable:4005)
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>

namespace libTools
{
	class CLdrHeader
	{
	public:
		CLdrHeader();
		~CLdrHeader() = default;

		// InlineHook(需要Hook的函数地址,代替被Hook函数的地址,返回原始函数的入口地址)
		static BOOL WINAPI InlineHook(_In_ void *OrgProc, _In_	void *NewProc, _Out_ void **RealProc);

		// UnInlineHook(需要恢复Hook的函数地址,原始函数的入口地址)
		static VOID WINAPI UnInlineHook(_In_ void *OrgProc, _In_ void *RealProc);
	private:
		static BOOL WINAPI WriteReadOnlyMemory(LPBYTE	lpDest, LPBYTE	lpSource, ULONG	Length);

		static BOOL WINAPI GetPatchSize(IN	void *Proc, IN	DWORD dwNeedSize, OUT LPDWORD lpPatchSize);
	};
}


#endif // !__LIBTOOLS_HOOKLIB_INLINEHOOK_LDRHEADER
