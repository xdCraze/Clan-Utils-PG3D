#pragma once

#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <string>

enum class ConsoleColor
{
    Reset = 7,
    Info = 3,
    Warning = 14,
    Error = 12
};

class Logger
{
public:
    static void setConsoleColor(ConsoleColor color);

    template<ConsoleColor Color>
    static void log(const char* format, ...);

private:
    static std::string formatMessage(const char* format, va_list args);
};