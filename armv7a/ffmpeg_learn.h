#pragma once
#define _DLLExport __attribute__((visibility("default"))) //定义该函数的dll
#include "MP42MP3Context.h"
extern "C" void _DLLExport Mp4ToMp3(char *srcFileName, char *dstFileName, char *logFileName);
//代表c风格的
extern "C" int _DLLExport Add(int x, int y);

extern "C" int _DLLExport Max(int x, int y);