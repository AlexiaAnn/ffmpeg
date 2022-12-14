#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif // ANDROID

#define DEFAULTGIFINPUTPIXFMT AV_PIX_FMT_RGB565
#define DEFAULTGIFPIXFMT AV_PIX_FMT_PAL8
class FilterContext
{
private:
	AVFilterGraph *filterGraph;
	AVFilterContext *buffersrcCont;
	AVFilterContext *buffersinkCont;
	AVFrame *sinkFrame;
	int ret;

public:
	FilterContext(AVCodecContext *codeCont);
	bool AddFrame(AVFrame *frame);
	bool FlushBuffer();
	AVFrame *GetFrame();
	int GetResult() const;
	~FilterContext();
};
