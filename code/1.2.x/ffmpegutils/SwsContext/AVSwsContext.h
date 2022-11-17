#pragma once
#include "SwsContextBase.h"
class AVSwsContext : public SwsContextBase
{
private:
	SwsContext *swsCont = nullptr;
	int ret = 0;

	//≤‚ ‘±‰¡ø
	clock_t start, end;
	float swsTime = 0;
	int frameCount = 0;
public:
	AVSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight,
				 AVPixelFormat enPixFormat, int enWidth, int enHeight);
	AVSwsContext(AVCodecContext *deCodecont,
				 AVPixelFormat enPixFormat, int enWidth, int enHeight);
	AVSwsContext(AVPixelFormat dePixFormat,
				 int deWidth, int deHeight, AVCodecContext *enCodecont);
	AVSwsContext(AVCodecContext *deCodecont, AVCodecContext *enCodecont);
	bool IsNeedRescale() const;
	bool RescaleVideoFrame(AVFrame *deVideoFrame, AVFrame *enVideoFrame) override;
	bool RescaleVideoFrame(AVFrame *deVideoFrame, EnCodecVideoContext &codeCont) override;
	void GetTimeInfo() const override;
	~AVSwsContext();
	int GetResult() const override;
};
