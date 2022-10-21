#pragma once
#include "AVSwsContext.h"
#include "AVSwrContext.h"
#include "EnCodecAudioContext.h"
#include "EnCodecVideoContext.h"
#include "OutFormatContext.h"
#define DEFAULTAUDIOCODECID AV_CODEC_ID_MP3
#define DEFAULTVIDEOCODECID AV_CODEC_ID_H264
class RecordMp4
{
private:
	AVSwsContext* swsCont;
	AVSwrContext* swrCont;
	EnCodecAudioContext* enAudioCont;
	EnCodecVideoContext* enVideoCont;
	OutFormatContext* outfmtCont;
	AVStream* audioStream;
	AVStream* videoStream;
	AVFrame* deAudioFrame;
	AVFrame* deVideoFrame;
	int ret;
public:
	RecordMp4(const char* dstFilepath,
			  int sampleRate,AVSampleFormat sampleFmt,AVChannelLayout chLayout,
			  AVPixelFormat dePixfmt,int fps,float bitRatePercent,int width,int height);
	bool WriteAVPreparition();
	bool WriteVideoToFile(void* data, int length);
	bool WriteAudioToFile(void* data, int length);
	bool FlushEnVideoCodecBuffer();
	bool FlushEnAudioCodecBuffer();
	bool WriteAVTailer();
	int GetResult() const;
	~RecordMp4();
};

