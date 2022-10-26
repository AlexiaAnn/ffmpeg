#pragma once
#include "InFormatContext.h"
#include "OutFormatContext.h"
#include "DeCodecContext.h"
#include "AVSwrContext.h"
#include "EnCodecAudioContext.h"
class ExtractAudio
{
private:
	InFormatContext* inFmtContPointer;
	DeCodecContext* deCodeContPointer;
	AVSwrContext* swrContPointer;
	EnCodecAudioContext* enCodeContPointer;
	OutFormatContext* outFmtContPointer;
	AVStream* audioStream;
	AVStream* deAudioStream;
	int ret;
public:
	ExtractAudio();
	ExtractAudio(const char* srcFilePath,const char* dstFilePath);
	void DoExtract();
	int GetResult() const;
	~ExtractAudio();
};

