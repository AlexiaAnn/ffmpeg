#pragma once
#ifdef WINDOWS
#include "../ffmpegutils/InFormatContext.h"
#include "../ffmpegutils/DeCodecContext.h"
#include "../ffmpegutils/SwsContext/AVSwsContext.h"
#include "../ffmpegutils/encodec/EnCodecVideoContext.h"
#include "../ffmpegutils/FilterContext.h"
#include "../ffmpegutils/OutFormatContext.h"
#endif // WINDOWS

#ifdef ANDROID
#include "ffmpegutils/InFormatContext.h"
#include "ffmpegutils/DeCodecContext.h"
#include "ffmpegutils/SwsContext/AVSwsContext.h"
#include "ffmpegutils/encodec/EnCodecVideoContext.h"
#include "ffmpegutils/FilterContext.h"
#include "ffmpegutils/OutFormatContext.h"
#endif // ANDROID

class VideoToGif
{
private:
	InFormatContext *infmtCont = nullptr;
	DeCodecContext *deCodeCont = nullptr;
	AVSwsContext *swsCont = nullptr;
	EnCodecVideoContext *enCodeCont = nullptr;
	FilterContext *filterCont = nullptr;
	OutFormatContext *outfmtCont = nullptr;
	AVStream *videoStream = nullptr;
	int pts = 0;
	// AVFrame* swsFrame;
	int ret = 0;

public:
	VideoToGif(const char *srcFilePath, const char *dstFilePath);
	int GetResult() const;
	bool DoConvert();
	~VideoToGif();
};
