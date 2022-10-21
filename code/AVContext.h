#pragma once
#include "VideoContext.h"
#include "AudioContext.h"

class AVContext : public VideoContext, public AudioContext
{
private:
    AVFormatContext *outAVFmtCtx;
    int ret;

public:
    AVContext();
    AVContext(const char *dstFilePath,
              int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout,
              AVPixelFormat dePixFormat, int fps, int width, int height);
    AVContext(const char *dstFilePath,
              int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout,
              AVPixelFormat dePixFormat, int fps, int deWidth, int deHeight, int enWidth, int enHeight);
    void WriteAVPreparition(const char *dstFilePath);
    void WriteAVTailer();
    int GetRet() const;
};