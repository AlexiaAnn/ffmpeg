#include "VideoEnCodecConfigBase.h"

const std::string VideoEnCodecConfigBase::encodeNames[4] = {"h264_hlmediacodec", "hevc_hlmediacodec", "libx264", "gif"};
VideoEnCodecConfigBase::VideoEnCodecConfigBase()
{
}

VideoEnCodecConfigBase::VideoEnCodecConfigBase(int width, int height, int fps, EncodeName encodeName)
{
	this->width = width;
	this->height = height;
	this->fps = fps;
}

AVCodecContext *VideoEnCodecConfigBase::GetContext()
{
	const AVCodec *codec = avcodec_find_encoder_by_name(encodeNames[int(encodeName)].c_str());
	AVCodecContext *context = AllocEncodecContext(encodeNames[int(encodeName)].c_str());
	if (context == nullptr)
	{
		return nullptr;
	}
	context->codec_id = codec->id;
	context->pix_fmt = GetPixFormatByCodecId(context->codec_id);
	context->codec_type = AVMEDIA_TYPE_VIDEO;
	context->width = width;
	context->height = height;
	// context->rc_buffer_size = (int)context->bit_rate;
	context->framerate.num = fps;
	context->framerate.den = 1;
	context->time_base.num = 1;
	context->time_base.den = fps;
	context->thread_count = 8;
	context->thread_type = FF_THREAD_FRAME;
	context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	return context;
}

VideoEnCodecConfigBase::~VideoEnCodecConfigBase() {}
