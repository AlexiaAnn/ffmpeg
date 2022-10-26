#pragma once
#include "InFormatContext.h"
#include "DeCodecContext.h"
#include "AVSwsContext.h"
class SeekVideo
{
private:
	InFormatContext* inFmtCont;
	DeCodecContext* deCodecont;
	AVSwsContext* swsContPointer;
	AVStream* videoStream;
	AVFrame* swsVideoFrame;
	int frameNumber;
	int fps;
	int ret;
	int curFrameIndex;
private:
	inline int PtsToFrameIndex(int64_t pts);
	inline int64_t FrameIndexToPts(int frameIndex);
	AVFrame* SeekFrameByPercent(float percent);
	AVFrame* GetNextFrame();
	void SeekFrameByFrameIndex(int frameIndex);
public:
	SeekVideo();
	SeekVideo(const char* srcFilePath);
	int GetVideoWidth() const;
	int GetVideoHeight() const;
	float GetDuration() const;
	void GetFrameDataByPercent(float percent,void* data,int length);
	int GetResult() const;
	void GetSomeInformation() const;
	~SeekVideo();
};

