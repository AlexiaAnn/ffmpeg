#pragma once
#ifdef WINDOWS

#include "../ffmpegutils/InFormatContext.h"
#include "../ffmpegutils/OutFormatContext.h"
#endif // WINDOWS

#ifdef ANDROID

#include "ffmpegutils/InFormatContext.h"
#include "ffmpegutils/OutFormatContext.h"
#endif // ANDROID

class ExtractBase
{
protected:
	InFormatContext *inFmtContPointer = nullptr;
	OutFormatContext *outFmtContPointer = nullptr;
	int ret;

public:
	ExtractBase();
	ExtractBase(const char *srcFilePath);
	virtual void DoExtract() = 0;
	int GetResult() const;
	~ExtractBase();
};
