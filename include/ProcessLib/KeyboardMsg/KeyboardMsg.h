#ifndef __LIBTOOLS_PROCESS_KEYBOARDMSG_KEYBOARDMSG_H__
#define __LIBTOOLS_PROCESS_KEYBOARDMSG_KEYBOARDMSG_H__

#include <Windows.h>

namespace libTools
{
	class CKeyboardMsg
	{
	public:
		CKeyboardMsg() = default;
		~CKeyboardMsg() = default;

		// 模拟按键
		static BOOL		SimulationKey(_In_  HWND hWnd, _In_ CHAR dwASCII, _In_opt_ DWORD dwTime = 10);

		// 获取虚拟码
		static DWORD	GetVirKey(_In_ CHAR dwASCII);

		// 模拟鼠标
		static BOOL		SimulationMouse(_In_ int x, _In_ int y);

		// 模拟鼠标
		static BOOL		SimulationRightMouse(_In_ int x, _In_ int y);

		// 发送文本
		static BOOL		SendAscii(_In_ WCHAR data, _In_  BOOL shift);

		// 发送文本
		static BOOL		SendUnicode(_In_ WCHAR data);

		// 模拟按键
		static BOOL		SendKey(_In_ WORD wVk);

		// 发送文本
		static BOOL		SendKeys(_In_ LPCWSTR data);

		// 发送文本
		static BOOL		SendKeys(_In_ LPCSTR data);
	private:

	};
}

#endif // !__LIBTOOLS_PROCESS_KEYBOARDMSG_KEYBOARDMSG_H__
