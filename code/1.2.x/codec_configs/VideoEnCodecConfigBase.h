#pragma once
#include "../utils/util.h"

enum EncodeName {
	H264HLMEDIACODEC = 0,
	H265HLMEDIACODEC,
	LIBX264
};
class VideoEnCodecConfigBase {
protected:
	static const std::string encodeNames[3];
	int width=720;
	int height=1280;
	int fps=25;
	EncodeName encodeName = H264HLMEDIACODEC;
public:
	VideoEnCodecConfigBase();
	VideoEnCodecConfigBase(int width,int height,int fps,EncodeName encodeName);
	virtual AVCodecContext* GetContext();
};