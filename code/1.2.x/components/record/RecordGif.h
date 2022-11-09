#pragma once
#include "ffmpegutils/SwsContext/AVSwsContext.h"
#include "ffmpegutils/encodec/EnCodecVideoContext.h"
#include "ffmpegutils/OutFormatContext.h"
#include "ffmpegutils/FilterContext.h"
class RecordGif
{
private:
	AVSwsContext *swsCont = nullptr;
	EnCodecVideoContext *enVideoCont = nullptr;
	OutFormatContext *outfmtCont = nullptr;
	FilterContext *filterCont = nullptr;
	AVStream *videoStream = nullptr;
	AVFrame *deVideoFrame = nullptr;
	bool isFirstFrame = true;
	int pts = 0;
	int ret = 0;
	int inFrameCount = 0;

public:
	RecordGif(const char *dstFilepath,
			  AVPixelFormat dePixfmt, int fps, float bitRatePercent, int width, int height, int presetLevel);
	int GetResult() const;
	bool WriteGIFPreparition();
	bool WriteVideoToFile(void *data, int length);
	bool FlushEnVideoCodecBuffer();
	bool WriteGIFTailer();
	~RecordGif();
};
