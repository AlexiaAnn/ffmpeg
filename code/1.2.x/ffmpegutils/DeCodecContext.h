#pragma once
#include "util.h"
#include "InFormatContext.h"
class DeCodecContext {
private:
    AVCodecContext* codecCont;
    AVFrame* deFrame;
    int streamIndex;
    int ret;
public:
    DeCodecContext(AVStream* stream);
    AVFrame* GetReceiveFrame();
    AVFrame* GetNextFrame(InFormatContext& infmtCont);
    bool SendPacket(AVPacket* packet);
    AVCodecContext* GetCodecContext() const;
    ~DeCodecContext();
    int GetResult() const;
};
