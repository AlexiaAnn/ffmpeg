#pragma once
#include "InFormatContext.h"
#include "DeCodecContext.h"
class ReadFileBase
{
protected:
	InFormatContext* inFmtCont;
	DeCodecContext* deCodeCont;
	int ret;
public:
	ReadFileBase(const char* srcFilePath, AVMediaType mediaType);
	int GetResult() const;
	~ReadFileBase();
};

