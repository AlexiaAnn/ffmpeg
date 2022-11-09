#pragma once
#include "utils/util.h"

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
