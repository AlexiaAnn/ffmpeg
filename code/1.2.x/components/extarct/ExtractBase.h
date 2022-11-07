#pragma once
#include "../ffmpegutils/InFormatContext.h"
#include "../ffmpegutils/OutFormatContext.h"
class ExtractBase
{
protected:
	InFormatContext* inFmtContPointer=nullptr;
	OutFormatContext* outFmtContPointer=nullptr;
	int ret;
public:
	ExtractBase();
	ExtractBase(const char* srcFilePath);
	virtual void DoExtract() = 0;
	int GetResult() const;
	~ExtractBase();
};

