#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#include "../ffmpegutils/encodec/EnCodecVideoContext.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#include "ffmpegutils/encodec/EnCodecVideoContext.h"
#endif // ANDROID


class SwsContextBase
{
public:
    virtual bool RescaleVideoFrame(AVFrame *deVideoFrame, AVFrame *enVideoFrame) = 0;
    virtual bool RescaleVideoFrame(AVFrame *deVideoFrame, EnCodecVideoContext &codeCont) = 0;
    virtual int GetResult() const = 0;
    virtual void GetTimeInfo() const = 0;
    virtual ~SwsContextBase() {}
};