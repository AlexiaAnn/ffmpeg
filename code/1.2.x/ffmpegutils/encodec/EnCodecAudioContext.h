#pragma once
#include "EnCodecContext.h"
#include "../OutFormatContext.h"
#define DEFAULTAUDIOCODECID AV_CODEC_ID_MP3
class EnCodecAudioContext :
    public EnCodecContext
{
private:
    int maxNbSamples=0;
protected:
    
    AVCodecContext* OpenEncodecContext(AVCodecID enCodecid);
    AVFrame* InitAudioFrame(AVCodecContext* codeCont,int dstNbSamples);
public:
    EnCodecAudioContext();
    EnCodecAudioContext(AVCodecID codecId);

    void ReAllocFrame(int dstNbSamples);
    bool EncodeFrame(OutFormatContext& outFmtCont, AVStream* outStream) override;
    bool EncodeFrame(OutFormatContext& outFmtCont, AVStream* outStream, AVFrame* enFrame) override;
    bool FlushBuffer(OutFormatContext& outFmtCont, AVStream* outStream) override;
    int GetNbSamplesOfFrameBuffer() const;
    AVFrame* GetEncodecFrame() const;
    ~EnCodecAudioContext();
};

