#pragma once
#ifdef WINDOWS
#include "../ffmpegutils/SwsContext/AVSwsContext.h"
#include "../ffmpegutils/AVSwrContext.h"
#include "../ffmpegutils/encodec/EnCodecAudioContext.h"
#include "../ffmpegutils/encodec/EnCodecVideoContext.h"
#include "../ffmpegutils/OutFormatContext.h"
#include "../ffmpegutils/SwsContext/SwsContextBase.h"
#include "../ffmpegutils/SwsContext/LibyuvSwsContext.h"
#include "../ffmpegutils/encodec/VideoEncoderThread.h"
#endif // WINDOWS

#ifdef ANDROID
#include "ffmpegutils/SwsContext/AVSwsContext.h"
#include "ffmpegutils/AVSwrContext.h"
#include "ffmpegutils/encodec/EnCodecAudioContext.h"
#include "ffmpegutils/encodec/EnCodecVideoContext.h"
#include "ffmpegutils/OutFormatContext.h"
#include "ffmpegutils/SwsContext/SwsContextBase.h"
#include "ffmpegutils/SwsContext/LibyuvSwsContext.h"
#include "ffmpegutils/encodec/VideoEncoderThread.h"
#endif // ANDROID

#include <mutex>
#include <condition_variable>
#include <queue>
#include <algorithm>
#define DEFAULTAUDIOCODECID AV_CODEC_ID_MP3
#define DEFAULTVIDEOCODECID AV_CODEC_ID_H264


class RecordMp4Thread
{
private:
	SwsContextBase* swsCont = nullptr;
	AVSwrContext* swrCont = nullptr;
	EnCodecAudioContext* enAudioCont = nullptr;
	EnCodecVideoContext* enVideoCont = nullptr;
	OutFormatContext* outfmtCont = nullptr;
	AVStream* audioStream = nullptr;
	AVStream* videoStream = nullptr;
	AVFrame* deAudioFrame = nullptr;
	clock_t start, end;
	float flipDurationAvg = 0;
	float rescaleDurationAvg = 0, encodeDurationAvg = 0;
	float allDurationAvg = 0;
	int iFrameCount = 0;
	int ret = 0;

public:
	RecordMp4Thread(const char* dstFilepath,
		int sampleRate, AVSampleFormat sampleFmt, AVChannelLayout chLayout,
		AVPixelFormat dePixfmt, int fps, float bitRatePercent, int width, int height, int crfMin, int crfMax, int presetLevel);
	bool WriteAVPreparition();
	bool WriteVideoToFile(void* data, int length);
	bool WriteAudioToFile(void* data, int length);
	bool FlushEnVideoCodecBuffer();
	bool FlushEnAudioCodecBuffer();
	bool WriteAVTailer();
	int GetResult() const;
	~RecordMp4Thread();
};
