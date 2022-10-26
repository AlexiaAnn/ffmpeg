#pragma once
#include "IncludeFFmpeg.h"
#include "util.h"
#define DEFAULTVIDEOCODECID AV_CODEC_ID_H264
#define DEFAULTPIXFORMAT AV_PIX_FMT_RGBA
#define DEFAULTDSTPIXFORMAT AV_PIX_FMT_YUV420P
#define DEFAULTCRFMAXVALUE 28
class VideoContext
{
protected:
    AVCodecContext *enVideoCodecCtx;
    AVFormatContext *outVideoFmtCtx;
    AVFrame *deVideoFrame;
    AVFrame *enVideoFrame;
    AVPacket *enVideoPacket;
    SwsContext *swsCtx;
    int pts, ret;
    int deWidth, deHeight;
    AVPixelFormat srcPixFormat;
    AVStream *outVideoStream;

protected:
    VideoContext();
    AVCodecContext *OpenVideoEncodecContext(AVCodecID codecId, int fps, int width, int height,float bitRatePercent);
    SwsContext *InitSwsContext(AVCodecContext *deCodecCtx, AVCodecContext *enCodecCtx);
    SwsContext *InitSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight, AVCodecContext *enCodecCtx);
    AVFrame *CreateVideoAVFrame(AVCodecContext *codecContext);
    int GetPixFormatChannelNumber() const;
    void RescaleDevideoFrame();
    void EncodeVideoFrame();
    bool VariableCheck();

public:
    //VideoContext(AVCodecID dstCodecId);
    VideoContext(AVCodecID codecId, int fps, float bitRatePercent, int width, int height);
    //VideoContext(AVPixelFormat dePixFormat, int fps, float bitRatePercent,int width, int height);
    VideoContext(AVPixelFormat dePixFormat, AVCodecID codecId, int fps, float bitRatePercent, int width, int height);
    VideoContext(AVPixelFormat dePixFormat, AVCodecID codecId, int fps, float bitRatePercent,int deWidth, int deHeight, int enWidth, int enHeight);
    // function region of writing to file
    void WriteVideoPreparition(const char *dstFilePath);
    void WriteVideoToFile(AVFrame* deVideoFrame);
    void WriteVideoToFile(void *data, int length);
    void WriteVideoToFile();
    void WriteVideoTailer();
    int Flip(unsigned char* src);
    void FlushEnVideoCodecBuffer();
    virtual ~VideoContext();
};