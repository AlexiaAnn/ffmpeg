#pragma once
#include "util.h"
#include "EnCodecVideoContext.h"
class AVSwsContext
{
private: 
	SwsContext* swsCont;
	int ret;
public:
	AVSwsContext(AVPixelFormat dePixFormat,int deWidth,int deHeight,AVPixelFormat enPixFormat,int enWidth,int enHeight);
	AVSwsContext(AVCodecContext* deCodecont, AVPixelFormat enPixFormat, int enWidth, int enHeight);
	AVSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight, AVCodecContext* enCodecont);
	AVSwsContext(AVCodecContext* deCodecont, AVCodecContext* enCodecont);
	bool IsNeedRescale() const;
	bool RescaleVideoFrame(AVFrame* deVideoFrame, AVFrame* enVideoFrame);
	bool RescaleVideoFrame(AVFrame* deVideoFrame, EnCodecVideoContext& codeCont);
	~AVSwsContext();
	int GetResult() const;
};
