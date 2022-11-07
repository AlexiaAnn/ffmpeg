#pragma once
#include "../ffmpegutils/AVSwsContext.h"
#include "../ffmpegutils/AVSwrContext.h"
#include "../ffmpegutils/encodec/EnCodecAudioContext.h"
#include "../ffmpegutils/encodec/EnCodecVideoContext.h"
#include "../ffmpegutils/OutFormatContext.h"
#include <mutex>
#define DEFAULTAUDIOCODECID AV_CODEC_ID_MP3
#define DEFAULTVIDEOCODECID AV_CODEC_ID_H264

class RecordMp4
{
private:
	AVSwsContext* swsCont = nullptr;
	AVSwrContext* swrCont = nullptr;
	EnCodecAudioContext* enAudioCont = nullptr;
	EnCodecVideoContext* enVideoCont = nullptr;
	OutFormatContext* outfmtCont = nullptr;
	AVStream* audioStream = nullptr;
	AVStream* videoStream = nullptr;
	AVFrame* deAudioFrame = nullptr;
	AVFrame* deVideoFrame = nullptr;
	int ret=0;
public:
	RecordMp4(const char* dstFilepath,
			  int sampleRate,AVSampleFormat sampleFmt,AVChannelLayout chLayout,
			  AVPixelFormat dePixfmt,int fps,float bitRatePercent,int width,int height, int crfMin, int crfMax,int presetLevel);
	bool WriteAVPreparition();
	bool WriteVideoToFile(void* data, int length);
	bool WriteAudioToFile(void* data, int length);
	bool FlushEnVideoCodecBuffer();
	bool FlushEnAudioCodecBuffer();
	bool WriteAVTailer();
	int GetResult() const;
	~RecordMp4();
};

