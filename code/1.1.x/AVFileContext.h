#pragma once
#include "VideoFileContext.h"
#include "AudioFileContext.h"

class AVFileContext : public VideoFileContext, public AudioFileContext
{
protected:
    AVFileContext();
    AVFormatContext *outAVFmtCtx;
    int ret;

public:
    AVFileContext(const char *srcFilePath);
    AVFileContext(int fps, int width, int height);
    AVFileContext(int deWidth, int deHeight, int fps, int width, int height);
    AVFileContext(AVCodecID audioDstCodecID,AVPixelFormat srcPixFormat, AVCodecID codecID, int deWidth, int deHeight, int fps, int width, int height);
    void WriteAVPreparition(const char *dstFilePath);
    void WriteAVTailer();
    bool ExtractAVToFile(const char *srcFilePath, const char *dstFilePath);
    virtual ~AVFileContext();
};