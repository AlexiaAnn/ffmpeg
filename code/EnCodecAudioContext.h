#pragma once
#include "EnCodecContext.h"
#include "OutFormatContext.h"
#define DEFAULTAUDIOCODECID AV_CODEC_ID_MP3
class EnCodecAudioContext :
    public EnCodecContext
{
private:
    AVFrame* frame;
    int maxNbSamples;
    int pts;
    AVPacket* packet;
protected:
    
    AVCodecContext* OpenEncodecContext(AVCodecID enCodecid);
    AVFrame* InitAudioFrame(AVCodecContext* codeCont,int dstNbSamples);
public:
    EnCodecAudioContext();
    EnCodecAudioContext(AVCodecID codecId);
    void ReAllocFrame(int dstNbSamples);
    bool EncodeAudioFrame(AVFormatContext* fmtCont,AVStream* outStream);
    bool EncodeAudioFrame(OutFormatContext& outFmtCont, AVStream* outStream);
    bool FlushBuffer(AVFormatContext* fmtCont, AVStream* outStream);
    bool FlushBuffer(OutFormatContext& outFmtCont, AVStream* outStream);
    int GetNbSamplesOfFrameBuffer() const;
    AVFrame* GetEncodecFrame() const;
    ~EnCodecAudioContext();
};

