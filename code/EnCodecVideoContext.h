#pragma once
#include "EnCodecContext.h"
#include "OutFormatContext.h"
#define DEFAULTCODECID AV_CODEC_ID_H264
#define DEFAULTCRFMAXVALUE 28
#define DEFAULTBITRATEPERCENT 0.2
class EnCodecVideoContext :
    public EnCodecContext
{
private:
    
protected:
    AVCodecContext* OpenEncodecContext(AVCodecID enCodecid, int width, int height, int fps, float bitRatePercent);
    AVFrame* CreateVideoFrame(const AVCodecContext* codeCont);
public:
    EnCodecVideoContext();
    EnCodecVideoContext(AVCodecID codecId,int width,int height,int fps,float bitRatePercent);

    bool EncodeFrame(OutFormatContext& outFmtCont, AVStream* outStream) override;
    bool EncodeFrame(OutFormatContext& outFmtCont, AVStream* outStream, AVFrame* enFrame) override;
    bool FlushBuffer(OutFormatContext& outFmtCont, AVStream* outStream) override;
    AVFrame* GetEncodecFrame() const;
    ~EnCodecVideoContext();
};

