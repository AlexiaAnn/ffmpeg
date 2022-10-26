#pragma once
#include "InFormatContext.h"
#include "OutFormatContext.h"
class ExtractBase
{
protected:
	InFormatContext* inFmtContPointer;
	OutFormatContext* outFmtContPointer;
	int ret;
public:
	ExtractBase();
	ExtractBase(const char* srcFilePath);
	virtual void DoExtract() = 0;
	int GetResult() const;
	~ExtractBase();
};

