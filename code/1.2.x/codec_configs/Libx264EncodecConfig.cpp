#include "Libx264EncodecConfig.h"

const std::string Libx264EncodecConfig::presetLevels[9]= { "ultrafast","superfast","veryfast","faster","fast","medium","slow","slower","veryslow" };

Libx264EncodecConfig::Libx264EncodecConfig(int width, int height, int fps, EncodeName encodeName, 
										   float bitRatePercent, int crfMin, int crfMax, enum PresetLevel level):
										   VideoEnCodecConfigBase(width,height,fps,encodeName)
{
	this->bitRatePercent = bitRatePercent;
	this->crfMin = crfMin;
	this->crfMax = crfMax;
	this->level = level;
	this->encodeName = encodeName;
}

AVCodecContext* Libx264EncodecConfig::GetContext() {
	
	AVCodecContext* context = VideoEnCodecConfigBase::GetContext();
	if (context == nullptr) return nullptr;

	const AVCodec* codec = avcodec_find_encoder_by_name(encodeNames[int(encodeName)].c_str());
	int64_t crf = crfMin + (1 - bitRatePercent) * (crfMax - crfMin);
	if (crf<0 || crf>DEFAULTCRFMAXVALUE) crf = 23;
	av_log_info("crf interval:[%d,%d]", crfMin, crfMax);
	av_log_info("bit rate percent from unity:%f,target crf:%ld\n", bitRatePercent, crf);
	av_opt_set_int(context->priv_data, "crf", crf, 0);
	//av_opt_set_int(context->priv_data, "qp", 18, 0);
	av_opt_set(context->priv_data, "preset", presetLevels[(int)level].c_str(), 0);
	av_log_info("presetlevel from unity:%d,the string is %s\n", (int)level, presetLevels[level].c_str());
	av_log_info("is opening video codec context\n");
	av_log_info("bit rate percent from unity:%f,target crf:%ld\n", bitRatePercent, crf);
	av_log_info("context bitrate:%ld\n", context->bit_rate);
	int ret = avcodec_open2(context, codec, nullptr);
	if (ret < 0)
	{
		av_log_error("could not open codec context\n");
		return nullptr;
	}
	av_log_info("open video codec context end\n");
	return context;
}
