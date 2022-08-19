#pragma once
#include "util.h"
void MyLogOutput(void *, int, const char *, va_list);
class MP42MP3Context
{
private:
    const char *srcFileName;
    const char *dstFileName;
    FILE *dstFilePtr;
    AVFormatContext *formatContext;
    int audioStreamIndex;
    AVCodecContext *deCdcCtx, *enCdcCtx;
    SwrContext *swrCtx;
    AVFrame *srcFrame, *dstFrame;
    AVPacket *dePacket, *enPacket;
    int max_dst_nb_samples;
    std::shared_ptr<spdlog::logger> logger;
    int dePacketCount = 0, deFrameCount = 0;
    int deAPacketCount = 0;
    int enPacketCount = 0, enFrameCount = 0;

private:
    MP42MP3Context();
    void EncodePcm();
    void DecodePacket(void (MP42MP3Context::*funcP)());
    void TempDecodePacket(void (MP42MP3Context::*funcP)());
    int ReciveFrame(void (MP42MP3Context::*funcP)());
    void ResampleFrame();
    void InitDstFrame(int64_t dst_nb_samples);
    void ResetDstFrame(int64_t dst_nb_samples);
    void Resample();
    int SetSwrContext();

public:
    MP42MP3Context(const char *srcFileStr, const char *dstFileStr);
    ~MP42MP3Context();
    void Mp42Pcm();
    void Mp42Mp3();
};