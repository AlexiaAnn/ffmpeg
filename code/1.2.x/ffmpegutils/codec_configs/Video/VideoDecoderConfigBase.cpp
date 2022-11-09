#include "VideoDecoderConfigBase.h"

const std::string VideoDecoderConfigBase::decoderNames[2] = { "h264_mediacodec","h264" };


VideoDecoderConfigBase::VideoDecoderConfigBase(VideoDecoderName decoderName, AVStream* stream)
{
	this->decoderName = decoderName;
	this->stream = stream;
}

AVCodecContext* VideoDecoderConfigBase::GetContext()
{
	AVCodecContext* context = OpenDecodecContextByStream(decoderNames[(int)decoderName].c_str(),stream);
	return context;
}
