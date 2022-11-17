#pragma once
#ifdef WINDOWS
#include "../ffmpegutils/SwsContext/AVSwsContext.h"
#include "../ffmpegutils/encodec/EnCodecVideoContext.h"
#include "../ffmpegutils/OutFormatContext.h"
#endif // WINDOWS

#ifdef ANDROID
#include "ffmpegutils/SwsContext/AVSwsContext.h"
#include "ffmpegutils/encodec/EnCodecVideoContext.h"
#include "ffmpegutils/OutFormatContext.h"
#endif // ANDROID
class RecordBase
{
private:
	AVSwsContext *swsCont = nullptr;
	EnCodecVideoContext *enCodecont = nullptr;
	OutFormatContext *outFmtCont = nullptr;

public:
	RecordBase();
};
