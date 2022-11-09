#pragma once
#include "ffmpegutils/AVSwsContext.h"
#include "ffmpegutils/encodec/EnCodecVideoContext.h"
#include "ffmpegutils/OutFormatContext.h"
class RecordBase
{
private:
	AVSwsContext *swsCont = nullptr;
	EnCodecVideoContext *enCodecont = nullptr;
	OutFormatContext *outFmtCont = nullptr;

public:
	RecordBase();
};
