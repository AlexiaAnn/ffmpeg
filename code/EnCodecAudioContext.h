#pragma once
#include "EnCodecContext.h"
#define DEFAULTAUDIOCODECID AV_CODEC_ID_MP3
class EnCodecAudioContext :
    public EnCodecContext
{
private:
    AVFrame* frame;
    int pts;
    AVPacket* packet;
protected:
    AVCodecContext* OpenEncodecContext(AVCodecID enCodecid);
    AVFrame* InitAudioFrame(AVCodecContext* codeCont,int dstNbSamples);
public:
    EnCodecAudioContext();
    EnCodecAudioContext(AVCodecID codecId);
    bool EncodeAudioFrame(AVFormatContext* fmtCont,AVStream* outStream);
    bool FlushBuffer(AVFormatContext* fmtCont, AVStream* outStream);
    AVFrame* GetEncodecFrame() const;
    ~EnCodecAudioContext();
};

