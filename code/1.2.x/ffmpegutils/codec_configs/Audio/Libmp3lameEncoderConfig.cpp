#include "Libmp3lameEncoderConfig.h"

Libmp3lameEncoderConfig::Libmp3lameEncoderConfig(AudioEncodeName encodeName):AudioEncoderConfigBase(encodeName)
{

}

AVCodecContext* Libmp3lameEncoderConfig::GetContext()
{
	av_log_info("trying config libmp3lame encoder...");
	AVCodecContext* context = AudioEncoderConfigBase::GetContext();
	if (context == nullptr) return nullptr;

	const AVCodec* codec = avcodec_find_encoder_by_name(encodeNames[int(encodeName)].c_str());
	context->bit_rate = 320000;
	context->sample_rate = 48000; // error prone
	av_channel_layout_copy(&context->ch_layout, &layout);

	context->sample_fmt = codec->sample_fmts[0];
	int ret = avcodec_open2(context, codec, nullptr);
	if (ret < 0)
	{
		av_log_error("could not open libmp3lame encoder\n");
		return nullptr;
	}
	av_log_info("config libmp3lame encoder success");
	return context;
}

Libmp3lameEncoderConfig::~Libmp3lameEncoderConfig()
{
}
