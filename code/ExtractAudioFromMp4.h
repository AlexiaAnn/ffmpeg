#pragma once
#include "util.h"
#include "FileContextBase.h"
class ExtractAudioFromMp4:public FileContextBase
{
protected:
    const static AVCodecID defaultCodecId;
    SwrContext* swrCtx;
    AVCodecContext* deAudioCodecCtx;
    AVCodecContext* enAudioCodecCtx;
    AVFrame* deAudioFrame;
    AVFrame* enAudioFrame;
    AVPacket* enAudioPacket;
    FILE* dstFilePtr;
    AVCodecID enCodecId;
    int maxDstNbSamples;
    int64_t dstNbSamples;
    int pts;
    AVFormatContext* outFmtCtx;

protected:
    AVCodecContext* OpenAudioEncodecContext(AVCodecID codecId);
    void InitAudioFrame(AVFrame*& frame, AVCodecContext* codecCtx, int dstNbSamples);
    void ResampleDeAudioFrame();
    virtual void EncodeAudioFrame();
    void DealAudioPacket() override;
    SwrContext* AllocSwrContext(AVCodecContext* deCodecCtx, AVCodecContext* enCodecCtx);
    SwrContext* AllocSwrContext(int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout, AVCodecContext* enCodecCtx);
    void SetAudioEssentialParameters(AVCodecContext*& codecCtx, AVCodecID codecId);
    void WriteFrameEnd();
    ExtractAudioFromMp4();

public:
    ExtractAudioFromMp4(const char* srcFilePath);
    ExtractAudioFromMp4(AVCodecID dstCodecId);
    ExtractAudioFromMp4(int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout);
    bool ExtractAudioToFile(const char* srcFilePath, const char* dstFilePath);
    void ResetAudioCodecId(AVCodecID codecId);
    void FlushEnAudioCodecBuffer();
    void PcmToAACFile(const char* srcFilePath, const char* dstFilepath);
    virtual ~ExtractAudioFromMp4();
};

