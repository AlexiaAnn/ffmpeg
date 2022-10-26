#pragma once
#include "InFormatContext.h"
#include "DeCodecContext.h"
#include "AVSwsContext.h"
#include "EnCodecVideoContext.h"
#include "FilterContext.h"
#include "OutFormatContext.h"
class VideoToGif
{
private:
	InFormatContext* infmtCont;
	DeCodecContext* deCodeCont;
	AVSwsContext* swsCont;
	EnCodecVideoContext* enCodeCont;
	FilterContext* filterCont;
	OutFormatContext* outfmtCont;
	AVStream* videoStream;
	int pts;
	//AVFrame* swsFrame;
	int ret;
public:
	VideoToGif(const char* srcFilePath,const char* dstFilePath);
	int GetResult() const;
	bool DoConvert();
	~VideoToGif();
};

