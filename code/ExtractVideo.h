#pragma once
#include "ExtractBase.h"
#include "DeCodecContext.h"
#include "AVSwsContext.h"
#include "EnCodecVideoContext.h"
class ExtractVideo:public ExtractBase
{
private:
	DeCodecContext* deCodeContPointer;
	AVSwsContext* swsContPointer;
	EnCodecVideoContext* enCodeContPointer;
	AVStream* deStream;
	AVStream* enStream;
public:
	ExtractVideo();
	ExtractVideo(const char* srcFilePath,const char* dstFilePath);
	void DoExtract() override;
	~ExtractVideo();
};

