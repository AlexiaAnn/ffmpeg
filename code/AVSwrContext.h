#pragma once
#include "util.h"
#include "EnCodecAudioContext.h"
class AVSwrContext
{
private:
	SwrContext* swrCont;
	int ret;
private:
	void ReAllocFrame(AVFrame* &frame,int dstNbSamples);
public:
	AVSwrContext();
	AVSwrContext(int deSampleRate, AVSampleFormat deSampleFormat, AVChannelLayout deChLayout, 
				 int enSampleRate, AVSampleFormat enSampleFormat, AVChannelLayout enChLayout);
	AVSwrContext(AVCodecContext* deAudioCodecCont, int enSampleRate, AVSampleFormat enSampleFormat, AVChannelLayout enChLayout);
	AVSwrContext(int deSampleRate, AVSampleFormat deSampleFormat, AVChannelLayout deChLayout, AVCodecContext* enAudioCodecCont);
	AVSwrContext(AVCodecContext* deAudioCodecCont, AVCodecContext* enAudioCodecCont);
	bool ResampleAudioFrame(AVFrame* deFrame,AVFrame*& enFrame);
	bool ResampleAudioFrame(AVFrame* deFrame, EnCodecAudioContext& codeCont);
	int GetResult() const;
	~AVSwrContext();
};

