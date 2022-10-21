#pragma once
#include "EnCodecContext.h"
#define DEFAULTCRFMAXVALUE 28
class EnCodecVideoContext :
    public EnCodecContext
{
private:
    AVFrame* frame;
    AVPacket* packet;
    int pts;
protected:
    AVCodecContext* OpenEncodecContext(AVCodecID enCodecid, int width, int height, int fps, float bitRatePercent);
    AVFrame* CreateVideoFrame(const AVCodecContext* codeCont);
public:
    EnCodecVideoContext();
    EnCodecVideoContext(AVCodecID codecId,int width,int height,int fps,float bitRatePercent);
    bool EncodeVideoFrame(AVFormatContext* fmtCont, AVStream* outStream);
    bool EncodeVideoFrame(AVFormatContext* fmtCont, AVStream* outStream,AVFrame* videoFrame);
    bool FlushBuffer(AVFormatContext* fmtCont, AVStream* outStream);
    AVFrame* GetEncodecFrame() const;
    ~EnCodecVideoContext();
};

