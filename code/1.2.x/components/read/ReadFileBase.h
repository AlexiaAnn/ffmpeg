#pragma once
#ifdef WINDOWS
#include "../ffmpegutils/InFormatContext.h"
#include "../ffmpegutils/DeCodecContext.h"
#endif // WINDOWS

#ifdef ANDROID
#include "ffmpegutils/InFormatContext.h"
#include "ffmpegutils/DeCodecContext.h"
#endif // ANDROID

class ReadFileBase
{
protected:
	InFormatContext *inFmtCont = nullptr;
	DeCodecContext *deCodeCont = nullptr;
	int ret = 0;

public:
	ReadFileBase(const char *srcFilePath, AVMediaType mediaType);
	int GetResult() const;
	~ReadFileBase();
};
