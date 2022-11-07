#pragma once
#include "ExtractBase.h"
#include "../ffmpegutils/DeCodecContext.h"
#include "../ffmpegutils/AVSwsContext.h"
#include "../ffmpegutils/encodec/EnCodecVideoContext.h"
class ExtractVideo:public ExtractBase
{
private:
	DeCodecContext* deCodeContPointer=nullptr;
	AVSwsContext* swsContPointer = nullptr;
	EnCodecVideoContext* enCodeContPointer = nullptr;
	AVStream* deStream = nullptr;
	AVStream* enStream = nullptr;
public:
	ExtractVideo();
	ExtractVideo(const char* srcFilePath,const char* dstFilePath);
	void DoExtract() override;
	~ExtractVideo();
};

