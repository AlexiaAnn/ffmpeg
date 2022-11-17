#pragma once
#ifdef WINDOWS
#include "../ffmpegutils/SwsContext/AVSwsContext.h"
#include "../ffmpegutils/encodec/EnCodecVideoContext.h"
#include "../ffmpegutils/OutFormatContext.h"
#include "../ffmpegutils/FilterContext.h"
#endif // WINDOWS

#ifdef ANDROID
#include "ffmpegutils/SwsContext/AVSwsContext.h"
#include "ffmpegutils/encodec/EnCodecVideoContext.h"
#include "ffmpegutils/OutFormatContext.h"
#include "ffmpegutils/FilterContext.h"
#include "ffmpegutils/SwsContext/LibyuvSwsContext.h"
#endif // ANDROID
class RecordGif
{
private:
	AVSwsContext* swsCont = nullptr;
	EnCodecVideoContext* enVideoCont = nullptr;
	OutFormatContext* outfmtCont = nullptr;
	FilterContext* filterCont = nullptr;
	AVStream* videoStream = nullptr;
	AVFrame* deVideoFrame = nullptr;
	bool isFirstFrame = true;
	int pts = 0;
	int ret = 0;
	int inFrameCount = 0;
	uint8_t* mTempData = nullptr;

public:
	RecordGif(const char* dstFilepath,
		AVPixelFormat dePixfmt, int fps, float bitRatePercent, int width, int height, int presetLevel);
	int GetResult() const;
	bool WriteGIFPreparition();
	bool WriteVideoToFile(void* data, int length);
	void FlipImage(uint8_t* srcData,uint8_t* tempData,int width,int height);
	bool FlushEnVideoCodecBuffer();
	bool WriteGIFTailer();
	~RecordGif();
};