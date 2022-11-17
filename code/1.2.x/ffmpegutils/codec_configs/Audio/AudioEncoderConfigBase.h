#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif // ANDROID

enum AudioEncodeName
{
	LIBMP3LAME = 0,
	LIBMP3MF = 1
};
class AudioEncoderConfigBase
{
protected:
	static const std::string encodeNames[2];
	AudioEncodeName encodeName = LIBMP3LAME;
	AVChannelLayout layout = {AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO}};

public:
	AudioEncoderConfigBase(AudioEncodeName encodeName);
	virtual AVCodecContext *GetContext();
	virtual ~AudioEncoderConfigBase() = 0;
};
