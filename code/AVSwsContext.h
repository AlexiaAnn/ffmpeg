#pragma once
#include "util.h"
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
	bool RescaleVideoFrame(AVFrame* deVideoFrame, AVFrame* enVideoFrame);
	~AVSwsContext();
	int GetResult() const;
};

