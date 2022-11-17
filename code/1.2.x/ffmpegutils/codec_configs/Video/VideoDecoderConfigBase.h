#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif // ANDROID


enum VideoDecoderName
{
	H264MEDIACODEC = 0,
	H264 = 1,
};
class VideoDecoderConfigBase
{
protected:
	static const std::string decoderNames[2];
	AVStream *stream = nullptr;
	VideoDecoderName decoderName = H264;

public:
	VideoDecoderConfigBase(VideoDecoderName decoderName, AVStream *stream);
	AVCodecContext *GetContext();
};
