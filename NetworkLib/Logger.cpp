#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

#include "Logger.h"


Logger::Logger() noexcept
{
	mStdErrHandle = GetStdHandle(STD_ERROR_HANDLE);
	mStdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::PrintConsole(Color color, const wchar* str, ...)
{
	if (nullptr == str)
	{
		return;
	}

	SetColor(true, color);

	time_t timer = time(NULL);

	struct tm t;
	localtime_s(&t, &timer);

	wprintf(
		L"[%04u-%02u-%02u %02u:%02u:%02u] ",
		t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec
	);

	va_list ap;
	va_start(ap, str);
	vwprintf(str, ap);
	va_end(ap);

	fflush(stdout);

	ResetColor(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::SetColor(bool isStdOut, Color color)
{
	static WORD WinColorFG[static_cast<uint8>(Color::COLOR_COUNT)] =
	{
		0,																			                // BLACK
		FOREGROUND_RED,																                // RED
		FOREGROUND_GREEN,															                // GREEN
		FOREGROUND_RED | FOREGROUND_GREEN,															// BROWN
		FOREGROUND_BLUE,															                // BLUE
		FOREGROUND_RED | FOREGROUND_BLUE,															// MAGENTA
		FOREGROUND_GREEN | FOREGROUND_BLUE,															// CYAN
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,										// GREY
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,									// YELLOW
		FOREGROUND_RED | FOREGROUND_INTENSITY,														// LRED
		FOREGROUND_GREEN | FOREGROUND_INTENSITY,									                // LGREEN
		FOREGROUND_BLUE | FOREGROUND_INTENSITY,														// LBLUE
		FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,									// LMAGENTA
		FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,									// LCYAN
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY					// WHITE
	};

	SetConsoleTextAttribute((isStdOut ? mStdOutHandle : mStdErrHandle), WinColorFG[static_cast<uint8>(color)]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::ResetColor(bool isStdOut)
{
	SetConsoleTextAttribute((isStdOut ? mStdOutHandle : mStdErrHandle), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}