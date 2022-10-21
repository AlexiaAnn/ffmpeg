#pragma once
#include "AVSwsContext.h"
#include "EnCodecVideoContext.h"
#include "OutFormatContext.h"
class RecordBase
{
private:
	AVSwsContext* swsCont;
	EnCodecVideoContext* enCodecont;
	OutFormatContext* outFmtCont;
public:
	RecordBase();
};

