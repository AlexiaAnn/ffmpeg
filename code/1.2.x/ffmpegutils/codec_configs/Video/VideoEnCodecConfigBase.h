#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif // ANDROID


enum EncodeName
{
	H264HLMEDIACODEC = 0,
	H265HLMEDIACODEC = 1,
	LIBX264 = 2,
	GIF = 3,
};
class VideoEnCodecConfigBase
{
protected:
	static const std::string encodeNames[4];
	int width = 720;
	int height = 1280;
	int fps = 25;
	EncodeName encodeName = H264HLMEDIACODEC;

public:
	VideoEnCodecConfigBase();
	VideoEnCodecConfigBase(int width, int height, int fps, EncodeName encodeName);
	virtual AVCodecContext *GetContext();
	virtual ~VideoEnCodecConfigBase() = 0;
};