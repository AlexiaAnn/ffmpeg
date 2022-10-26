#pragma once
#include "FileContextBase.h"
#include "VideoContext.h"
class VideoFileContext : virtual public FileContextBase, public VideoContext
{
protected:
    AVCodecContext *deVideoCodecCtx;
    int ret;

protected:
    VideoFileContext();
    void DealVideoPacket() override;
    virtual void DealDeVideoFrame();
    bool VariableCheck(const char *srcFilePath);

public:
    VideoFileContext(int fps, int width, int height);
    VideoFileContext(int deWidth, int deHeight, int fps, int width, int height);
    VideoFileContext(AVPixelFormat srcPixelFormat, AVCodecID dstCodecId, int fps, int width, int height);
    VideoFileContext(AVPixelFormat srcPixelFormat, AVCodecID dstCodecId, int deWidth, int deHeight, int fps, int width, int height);
    bool ExtractVideoToFile(const char *srcFilePath, const char *dstFilePath);
    int GetRet() const;
    virtual ~VideoFileContext();
};