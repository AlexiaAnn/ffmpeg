#pragma once
#include "utils/util.h"
#include "ffmpegutils/OutFormatContext.h"
class EnCodecContext
{
protected:
	AVCodecContext *codecCont = nullptr;
	AVFrame *frame = nullptr;
	AVPacket *packet = nullptr;
	int ret = 0;
	int pts = 0;

public:
	EnCodecContext();
	int GetResult() const;
	AVCodecContext *GetAVCodecContext() const;
	virtual bool EncodeFrame(OutFormatContext &outFmtCont, AVStream *outStream) = 0;
	virtual bool EncodeFrame(OutFormatContext &outFmtCont, AVStream *outStream, AVFrame *enFrame) = 0;
	virtual bool FlushBuffer(OutFormatContext &outFmtCont, AVStream *outStream) = 0;
	virtual ~EnCodecContext();
};
