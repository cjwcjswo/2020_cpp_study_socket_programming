#include <Windows.h>
//#include <WinBase.h>
//#include <ConsoleApi2.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

#include "Logger.h"


//TODO 최흥배
// 채팅 서버 이후에 작업합니다.
// 파일 쓰기 기능을 추가하고, 특히 스레드를 사용하여 로그를 남기도록 합니다. 즉 로그 전용 스레드를 만듭니다
// OK(채팅서버 이후 작업 예정)
Logger::Logger() noexcept
{
	mStdErrHandle = GetStdHandle(STD_ERROR_HANDLE);
	mStdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::PrintCurrentTime()
{
	time_t timer = time(NULL);

	struct tm t;
	localtime_s(&t, &timer);

	wprintf(
		L"[%04u-%02u-%02u %02u:%02u:%02u] ",
		t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec
	);
}

//TODO 최흥배 
// 스레드 문제가 있습니다. 콘솔 출력 함수 자체는 스레드세이프 합니다.
// 그런데 문제는 아래 함수에서 시간을 찍고, 다음에 에러 메시지를 찍고 있습니다.
// 이경우 복수 스레드에서 동시에 호출하면 시간:메시지가 아닌 시간:시간:메시지:메시지 처럼 찍힐 수도 있습니다.
// 시간과 메시지를 한번의 콘솔 출력으로 찍도로 하던가, 스레드 세이프하게 동작하도록 변경해야 합니다.
// 두개 이상의 스레드에서 PrintConsole를 호출하면 SetColor, PrintCurrentTime, vwprintf 이 함수들의 순서가 뒤죽 박죽으로 호출됩니다 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::PrintConsole(Color color, const wchar* str, ...)
{
	if (nullptr == str)
	{
		return;
	}

	SetColor(true, color);

	PrintCurrentTime();

	va_list ap;
	va_start(ap, str);
	vwprintf(str, ap);
	va_end(ap);

	fflush(stdout);

	ResetColor(true);
}

void Logger::PrintConsole(Color color, const char* str, ...)
{
	if (nullptr == str)
	{
		return;
	}

	SetColor(true, color);

	PrintCurrentTime();

	va_list ap;
	va_start(ap, str);
	vprintf(str, ap);
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