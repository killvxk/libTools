#ifndef __LIBTOOLS_PROCESS_THREAD_THREAD_H__
#define __LIBTOOLS_PROCESS_THREAD_THREAD_H__

#include <Windows.h>

namespace libTools
{
	class CThread
	{
	public:
		CThread() = default;
		~CThread() = default;

		static DWORD GetMainThreadId();
	private:

	};
}

#endif // !__LIBTOOLS_PROCESS_THREAD_THREAD_H__
