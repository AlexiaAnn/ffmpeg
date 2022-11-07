#include "IncludeFFmpeg.h"
void UnityLogCallbackFunc(void* ptr, int level, const char* fmt, va_list vl)
{
    try {
        char acLogStr[2048];
        vsprintf(acLogStr, fmt, vl);
        Debug::Log(acLogStr);
    }
    catch (const std::exception& e) {
        Debug::Log(e.what());
    }
}
void LogCallbackTotxt(void* ptr, int level, const char* fmt, va_list vl)
{
    FILE* fp = fopen("f:/ffmpeglog.txt", "a+");
    if (fp)
    {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}
void WindowsCallbackFunc(void* ptr, int level, const char* fmt, va_list vl)
{
    char acLogStr[10240];
    vsprintf(acLogStr, fmt, vl);
    std::cout << acLogStr ;
}
void (*Debug::LogFunPtr)(char* message, int iSize);
void (*Debug::LogErrorFunPtr)(char* message, int iSize);
void Debug::Log(const char* fmt, ...)
{
    if (Debug::LogFunPtr == nullptr)return;
    char acLogStr[2048]; // = { 0 }; error prone
    va_list ap;
    va_start(ap, fmt);
    vsprintf(acLogStr, fmt, ap);
    va_end(ap);
    Debug::LogFunPtr(acLogStr, strlen(acLogStr));
}

void Debug::LogError(const char* msg, ...)
{
    if (Debug::LogErrorFunPtr == nullptr)return;
    char acLogStr[2048]; // = { 0 }; error prone
    va_list ap;
    va_start(ap, msg);
    vsprintf(acLogStr, msg, ap);
    va_end(ap);
    Debug::LogErrorFunPtr(acLogStr, strlen(acLogStr));
}
