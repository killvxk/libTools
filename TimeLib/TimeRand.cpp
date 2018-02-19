#include "TimeRand.h"
#include "TimeTick.h"

int libTools::CTimeRand::GetRand(_In_ int nMinValue, _In_ int nMaxValue)
{
	static UINT uSeed = static_cast<UINT>(CTimeTick::_GetTickCount64());
	srand(static_cast<UINT>(uSeed));
	uSeed = static_cast<UINT>(rand());
	return (nMinValue + uSeed % (nMaxValue - nMinValue + 1));
}

