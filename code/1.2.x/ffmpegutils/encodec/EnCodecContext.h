#pragma once
#include "util.h"
#include "OutFormatContext.h"
class EnCodecContext
{
protected:
	AVCodecContext* codecCont;
	AVFrame* frame;
	AVPacket* packet;
	int ret;
	int pts;
	
	
public:
	EnCodecContext();
	int GetResult() const;
	AVCodecContext* GetAVCodecContext() const;
	virtual bool EncodeFrame(OutFormatContext& outFmtCont, AVStream* outStream) = 0;
	virtual bool EncodeFrame(OutFormatContext& outFmtCont, AVStream* outStream,AVFrame* enFrame) = 0;
	virtual bool FlushBuffer(OutFormatContext& outFmtCont, AVStream* outStream) = 0;
	virtual ~EnCodecContext();
};

