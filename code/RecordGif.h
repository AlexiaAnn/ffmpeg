#pragma once
#include "AVSwsContext.h"
#include "EnCodecVideoContext.h"
#include "OutFormatContext.h"
#include "FilterContext.h"
class RecordGif
{
private:
	AVSwsContext* swsCont;
	EnCodecVideoContext* enVideoCont;
	OutFormatContext* outfmtCont;
	FilterContext* filterCont;
	AVStream* videoStream;
	AVFrame* deVideoFrame;
	int pts;
	int ret;
public:
	RecordGif(const char* dstFilepath,
			  AVPixelFormat dePixfmt, int fps, float bitRatePercent, int width, int height);
	int GetResult() const;
	bool WriteGIFPreparition();
	bool WriteVideoToFile(void* data, int length);
	bool FlushEnVideoCodecBuffer();
	bool WriteGIFTailer();
	~RecordGif();
};

