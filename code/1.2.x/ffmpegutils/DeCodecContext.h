#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif // ANDROID

#include "InFormatContext.h"
class DeCodecContext
{
private:
    AVCodecContext *codecCont = nullptr;
    AVFrame *deFrame = nullptr;
    int streamIndex = 0;
    int ret = 0;

public:
    DeCodecContext(AVStream *stream);
    AVFrame *GetReceiveFrame();
    AVFrame *GetNextFrame(InFormatContext &infmtCont);
    bool SendPacket(AVPacket *packet);
    AVCodecContext *GetCodecContext() const;
    ~DeCodecContext();
    int GetResult() const;
};
