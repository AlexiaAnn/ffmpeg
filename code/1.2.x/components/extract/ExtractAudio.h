#pragma once
#include "ffmpegutils/InFormatContext.h"
#include "ffmpegutils/OutFormatContext.h"
#include "ffmpegutils/DeCodecContext.h"
#include "ffmpegutils/AVSwrContext.h"
#include "ffmpegutils/encodec/EnCodecAudioContext.h"
class ExtractAudio
{
private:
	InFormatContext *inFmtContPointer = nullptr;
	DeCodecContext *deCodeContPointer = nullptr;
	AVSwrContext *swrContPointer = nullptr;
	EnCodecAudioContext *enCodeContPointer = nullptr;
	OutFormatContext *outFmtContPointer = nullptr;
	AVStream *audioStream = nullptr;
	AVStream *deAudioStream = nullptr;
	int ret = 0;

public:
	ExtractAudio();
	ExtractAudio(const char *srcFilePath, const char *dstFilePath);
	void DoExtract();
	int GetResult() const;
	~ExtractAudio();
};
