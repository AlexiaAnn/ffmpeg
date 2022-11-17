#pragma once
#ifdef WINDOWS
#include "../ffmpegutils/InFormatContext.h"
#include "../ffmpegutils/DeCodecContext.h"
#include "../ffmpegutils/SwsContext/AVSwsContext.h"
#endif // WINDOWS

#ifdef ANDROID
#include "ffmpegutils/InFormatContext.h"
#include "ffmpegutils/DeCodecContext.h"
#include "ffmpegutils/SwsContext/AVSwsContext.h"
#endif // ANDROID

class SeekVideo
{
private:
	InFormatContext *inFmtCont = nullptr;
	DeCodecContext *deCodecont = nullptr;
	AVSwsContext *swsContPointer = nullptr;
	AVStream *videoStream = nullptr;
	AVFrame *swsVideoFrame = nullptr;
	int frameNumber = 0;
	int fps = 0;
	int ret = 0;
	int curFrameIndex = 0;

private:
	inline int PtsToFrameIndex(int64_t pts);
	inline int64_t FrameIndexToPts(int frameIndex);
	AVFrame *SeekFrameByPercent(float percent);
	AVFrame *GetNextFrame();
	void SeekFrameByFrameIndex(int frameIndex);

public:
	SeekVideo();
	SeekVideo(const char *srcFilePath);
	int GetVideoWidth() const;
	int GetVideoHeight() const;
	float GetDuration() const;
	void GetFrameDataByPercent(float percent, void *data, int length);
	int GetResult() const;
	void GetSomeInformation() const;
	~SeekVideo();
};
