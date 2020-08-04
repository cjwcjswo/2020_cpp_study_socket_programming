#pragma once

#include <Windows.h>
#include "PrimitiveTypes.h"


enum class Color : uint8
{
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    GREY,
    YELLOW,
    LRED,
    LGREEN,
    LBLUE,
    LMAGENTA,
    LCYAN,
    WHITE,
    COLOR_COUNT
};


class Logger
{
public:
    Logger() noexcept;
    ~Logger() = default;


private:
    HANDLE mStdOutHandle;
    HANDLE mStdErrHandle;


private:
    void PrintCurrentTime();


public:
    void PrintConsole(Color color, const wchar* str, ...);
    void PrintConsole(Color color, const char* str, ...);
    void SetColor(bool isStdOut, Color color);
    void ResetColor(bool isStdOut);
};

inline Logger* GLogger = new Logger();
