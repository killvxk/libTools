#ifndef __LIBTOOLS_GDILIB_GDISCREENSHOT_H__
#define __LIBTOOLS_GDILIB_GDISCREENSHOT_H__

#include <Windows.h>
#include <string>
#include <memory>

namespace libTools
{
#define SetResDeleter(VarName, Deleter) CResManager<decltype(VarName),std::function<VOID(decltype(VarName)&)>> VarName##Manager(VarName,Deleter)

	template<typename ResHandle, typename Deleter>
	class CResManager
	{
	public:
		CResManager(_In_ ResHandle& ResHandle_, _In_ Deleter fnDeletePtr) : m_ResHandle(ResHandle_), m_fnDeletePtr(fnDeletePtr)
		{

		}
		~CResManager()
		{
			m_fnDeletePtr(m_ResHandle);
		}

		CResManager& operator=(CONST CResManager&) = delete;
	private:
		ResHandle& m_ResHandle;
		Deleter m_fnDeletePtr;
	};

	class CGdiScreenshot
	{
	public:
		CGdiScreenshot() = default;
		~CGdiScreenshot() = default;

		static BOOL Screenshot(_In_ HWND hWnd, _In_ CONST std::wstring& wsPath);

		static BOOL GetScreenShotContent(_In_ HWND hWnd, _Out_ std::shared_ptr<BYTE>& PicContentPtr, _Out_ UINT& uSize);
	private:

	};
}

#endif // !__LIBTOOLS_GDILIB_GDISCREENSHOT_H__
