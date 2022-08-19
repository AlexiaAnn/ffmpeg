#pragma once
#include "ContextBase.h"
class AudioContext : public ContextBase
{
protected:
    SwrContext* swrCtx;
    AVCodecContext* deAudioCodecCtx;
    AVCodecContext* enAudioCodecCtx;
    AVFrame* deAudioFrame;
    AVFrame* enAudioFrame;
    AVPacket* enAudioPacket;
    FILE* dstFilePtr;
    AVCodecID enCodecId;
    int maxDstNbSamples;

protected:
    AVCodecContext* OpenAudioEncodecContext(AVCodecID codecId);
    void InitAudioFrame(AVFrame*& frame, AVCodecContext* codecCtx, int dstNbSamples);
    void ResampleDeAudioFrame();
    void EncodeAudioFrame();
    void DealAudioPacket(AVPacket* packet) override;
    SwrContext* AllocSwrContext(AVCodecContext* deCodecCtx, AVCodecContext* enCodecCtx);
    void SetAudioEssentialParameters(AVCodecContext*& codecCtx, AVCodecID codecId);

public:
    AudioContext() = delete;
    AudioContext(AVCodecID dstCodecId);
    bool ExtractAudioToFile(const char* srcFilePath, const char* dstFilePath);
    void ResetAudioCodecId(AVCodecID codecId);
    virtual ~AudioContext();
};
