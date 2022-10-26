#pragma once
#include "VideoFileContext.h"
class SeekComponent :public VideoFileContext {
private:
	AVStream* videoStream;
	int curFrameIndex;
	int frameNumber;
	int ret;
private:
	inline int PtsToFrameIndex(int64_t pts);
	inline int64_t FrameIndexToPts(int frameIndex);
	
	void SeekFrameByFrameIndex(int frameIndex);
	AVFrame* GetNextFrame();
public:
	SeekComponent(const char* srcFilePath);
	int GetVideoWidth() const;
	int GetVideoHeight() const;
	double GetDuration() const;
	void GetFrameDataByPercent(float percent,void* data,int length);
	AVFrame* SeekFrameByPercent(float percent);
	int GetRet() const;
	~SeekComponent();
};