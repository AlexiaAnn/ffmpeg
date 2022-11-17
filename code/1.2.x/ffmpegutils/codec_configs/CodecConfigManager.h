#pragma once
#ifdef WINDOWS

#endif // WINDOWS
#ifdef ANDROID

#endif // ANDROID
#include "Video/Libx264EncodecConfig.h"
#include "Video/EnMediaCodecConfig.h"
#include "Audio/Libmp3lameEncoderConfig.h"
class CodecConfigManager
{
public:
	static AVCodecContext *GetVideoEncoder(EncodeName encodeName, int width, int height, int fps,
										   float bitRatePercent = 0.2, int crfMin = 18, int crfMax = 28, enum PresetLevel level = ULTRAFAST,
										   int in_timeout = 1000, int out_timtout = 1000, int in_timeout_times = 3, int out_timeout_times = 3);
	static AVCodecContext *GetAudioEncoder(AudioEncodeName encodeName);
};
