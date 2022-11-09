#include "AudioEncoderConfigBase.h"

const std::string AudioEncoderConfigBase::encodeNames[2] = { "libmp3lame","mp3_mf"};
AudioEncoderConfigBase::AudioEncoderConfigBase(AudioEncodeName encodeName)
{
	this->encodeName = encodeName;
}

AVCodecContext* AudioEncoderConfigBase::GetContext()
{
	const AVCodec* codec = avcodec_find_encoder_by_name(encodeNames[int(encodeName)].c_str());
	AVCodecContext* context = AllocEncodecContext(encodeNames[int(encodeName)].c_str());
	if (context == nullptr) {
		return nullptr;
	}
	return context;
}

AudioEncoderConfigBase::~AudioEncoderConfigBase() {}
