#pragma once
#define _DLLExport __attribute__((visibility("default"))) //定义该函数的dll
#include "MP42MP3Context.h"
#include <thread>
MP42MP3Context *context;
extern "C" void _DLLExport Mp4ToMp3(char *srcFileName, char *dstFileName, LogFunc func);
extern "C" void _DLLExport InitCSharpDelegate(void (*Log)(char *message, int iSize));