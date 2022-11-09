#include "CodecConfigManager.h"

AVCodecContext *CodecConfigManager::GetVideoEncoder(EncodeName encodeName, int width, int height, int fps,
													float bitRatePercent, int crfMin, int crfMax, enum PresetLevel level,
													int in_timeout, int out_timtout, int in_timeout_times, int out_timeout_times)
{
	AVCodecContext *context = nullptr;
	if (encodeName == EncodeName::H264HLMEDIACODEC)
	{

		EnMediaCodecConfig mediaConfig(width, height, fps, EncodeName::H264HLMEDIACODEC, bitRatePercent, in_timeout, out_timtout, in_timeout_times, out_timeout_times);
		context = mediaConfig.GetContext();
		if (context != nullptr)
			return context;
		av_log_info("trying config another video encoder");
		Libx264EncodecConfig lib264Config(width, height, fps, EncodeName::LIBX264, bitRatePercent, crfMin, crfMax, level);
		context = lib264Config.GetContext();
	}
	else if (encodeName == EncodeName::LIBX264)
	{
		Libx264EncodecConfig lib264Config(width, height, fps, EncodeName::LIBX264, bitRatePercent, crfMin, crfMax, level);
		context = lib264Config.GetContext();
	}
	else if (encodeName == EncodeName::GIF)
	{
		Libx264EncodecConfig lib264Config(width, height, fps, EncodeName::GIF, bitRatePercent, crfMin, crfMax, level);
		context = lib264Config.GetContext();
	}

	if (context != nullptr)
		return context;
	return nullptr;
}

AVCodecContext *CodecConfigManager::GetAudioEncoder(AudioEncodeName encodeName)
{
	AVCodecContext *context = nullptr;
	Libmp3lameEncoderConfig mp3Config(encodeName);
	context = mp3Config.GetContext();
	if (context != nullptr)
		return context;
	av_log_info("trying config another audio encoder");
	return nullptr;
}
