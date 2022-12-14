#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif // ANDROID

#include "encodec/EnCodecAudioContext.h"
class AVSwrContext
{
private:
	SwrContext *swrCont = nullptr;
	int ret = 0;

	//???Ա???
	clock_t start, end;
	int swsTime = 0;
private:
	void ReAllocFrame(AVFrame *&frame, int dstNbSamples);

public:
	AVSwrContext();
	AVSwrContext(int deSampleRate, AVSampleFormat deSampleFormat, AVChannelLayout deChLayout,
				 int enSampleRate, AVSampleFormat enSampleFormat, AVChannelLayout enChLayout);
	AVSwrContext(AVCodecContext *deAudioCodecCont, int enSampleRate, AVSampleFormat enSampleFormat, AVChannelLayout enChLayout);
	AVSwrContext(int deSampleRate, AVSampleFormat deSampleFormat, AVChannelLayout deChLayout, AVCodecContext *enAudioCodecCont);
	AVSwrContext(AVCodecContext *deAudioCodecCont, AVCodecContext *enAudioCodecCont);
	bool ResampleAudioFrame(AVFrame *deFrame, AVFrame *&enFrame);
	bool ResampleAudioFrame(AVFrame *deFrame, EnCodecAudioContext &codeCont);
	int GetResult() const;
	~AVSwrContext();
};
