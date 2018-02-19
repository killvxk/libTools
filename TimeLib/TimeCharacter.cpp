#include "TimeCharacter.h"
#include <vector>
#include <time.h> // mktime()
#include <include/CharacterLib/Character.h>

#pragma comment(lib,"CharacterLib.lib")

bool libTools::CTimeCharacter::TimeContent::operator==(_In_ CONST TimeContent& Content) CONST
{
	return GetTimeValue(SysTime) == GetTimeValue(Content.SysTime);
}

bool libTools::CTimeCharacter::TimeContent::operator<(_In_ CONST TimeContent& Content) CONST
{
	return GetTimeValue(SysTime) < GetTimeValue(Content.SysTime);
}

bool libTools::CTimeCharacter::TimeContent::operator<=(_In_ CONST TimeContent& Content) CONST
{
	return GetTimeValue(SysTime) <= GetTimeValue(Content.SysTime);
}

bool libTools::CTimeCharacter::TimeContent::operator>(_In_ CONST TimeContent& Content) CONST
{
	return GetTimeValue(SysTime) > GetTimeValue(Content.SysTime);
}

bool libTools::CTimeCharacter::TimeContent::operator>=(_In_ CONST TimeContent& Content) CONST
{
	return GetTimeValue(SysTime) >= GetTimeValue(Content.SysTime);
}

bool libTools::CTimeCharacter::TimeContent::operator!=(_In_ CONST TimeContent& Content) CONST
{
	return GetTimeValue(SysTime) != GetTimeValue(Content.SysTime);
}

LONGLONG libTools::CTimeCharacter::DateDiff(_In_ CONST TimeContent& Time1, _In_ CONST TimeContent& Time2, CTimeTick::em_TimeTick emTimeTick)
{
	LONGLONG TimeValue = abs(GetTimeValue(Time1.SysTime) - GetTimeValue(Time2.SysTime));

	switch (emTimeTick)
	{
	case CTimeTick::em_TimeTick::em_TimeTick_Hour:
		return TimeValue / 3600;
	case CTimeTick::em_TimeTick::em_TimeTick_Minute:
		return TimeValue / 60;
	case CTimeTick::em_TimeTick::em_TimeTick_Second:
		return TimeValue;
	default:
		::MessageBoxW(NULL, L"UnSupport emTimeTick Other Type!", L"Error", NULL);
		break;
	}
	return 0;
}

BOOL libTools::CTimeCharacter::GetTimeContent_By_Text(_In_ CONST std::wstring& wsText, _Out_opt_ TimeContent& TimeInfo)
{
	//GetTimeContent_By_Text(L"2017-1-1 1:1:1" | L"2017年1月1日 6时3分4秒", Return)
	auto ConvertTextPtrToWord = [](CONST std::wstring& wsParam)
	{
		return static_cast<WORD>(std::stoi(wsParam));
	};


	std::vector<std::wstring> Vec;
	if (wsText.find(L"-") != -1)
	{
		if (!CCharacter::SplitFormatText(wsText, L"*-*-* *:*:*", Vec) || Vec.size() != 6)
		{
			return FALSE;
		}
	}
	else if (wsText.find(L"年") != -1)
	{
		if (!CCharacter::SplitFormatText(wsText, L"*年*月*日 *时*分*秒", Vec) || Vec.size() != 6)
		{
			return FALSE;
		}
	}
	else
	{
		// Invalid Format
		return FALSE;
	}


	TimeInfo.SysTime.wYear = static_cast<WORD>(std::stoi(Vec.at(0)));
	TimeInfo.SysTime.wMonth = static_cast<WORD>(std::stoi(Vec.at(1)));
	TimeInfo.SysTime.wDay = static_cast<WORD>(std::stoi(Vec.at(2)));
	TimeInfo.SysTime.wHour = static_cast<WORD>(std::stoi(Vec.at(3)));
	TimeInfo.SysTime.wMinute = static_cast<WORD>(std::stoi(Vec.at(4)));
	TimeInfo.SysTime.wSecond = static_cast<WORD>(std::stoi(Vec.at(5)));
	return TRUE;
}

LONGLONG libTools::CTimeCharacter::GetTimeValue(_In_ CONST SYSTEMTIME& SysTime)
{
	struct tm t = { SysTime.wSecond, SysTime.wMinute, SysTime.wHour, SysTime.wDay, SysTime.wMonth - 1, SysTime.wYear - 1900, SysTime.wDayOfWeek, 0 ,0 };
	return mktime(&t);
}
