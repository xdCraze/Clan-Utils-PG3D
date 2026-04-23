#include "Logger.hpp"

void Logger::setConsoleColor(ConsoleColor color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color));
}

std::string Logger::formatMessage(const char* format, va_list args)
{
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    return std::string(buffer);
}

template<ConsoleColor Color>
void Logger::log(const char* format, ...)
{
    setConsoleColor(Color);

    const char* prefix = (Color == ConsoleColor::Info ? "INFO" : (Color == ConsoleColor::Error ? "ERROR" : "WARNING"));
    printf("[%s] ", prefix);

    va_list args;
    va_start(args, format);
    std::string message = formatMessage(format, args);
    va_end(args);

    printf("%s\n", message.c_str());
    setConsoleColor(ConsoleColor::Reset);
}

template void Logger::log<ConsoleColor::Info>(const char* format, ...);
template void Logger::log<ConsoleColor::Warning>(const char* format, ...);
template void Logger::log<ConsoleColor::Error>(const char* format, ...);