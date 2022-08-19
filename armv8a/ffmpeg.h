#pragma once
#define _DLLExport __attribute__((visibility("default"))) //定义该函数的dll
#include "AudioContext.h"
#include "VideoContext.h"
#include <thread>
VideoContext *videoContext = nullptr;
std::thread frameConsumeThread;
extern "C" void _DLLExport Mp4ToMp3(char *srcFileName, char *dstFileName, LogFunc func);
extern "C" void _DLLExport InitCSharpDelegate(void (*Log)(char *message, int iSize));
extern "C" bool _DLLExport RecordMP4Start(const char *dstFilePath, int width, int height, int fps);
extern "C" void _DLLExport WriteMP4Frame(void *dataPtr, int length);
extern "C" void _DLLExport RecordMP4End();