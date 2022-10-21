#include "VideoFileContext.h"
void VideoFileContext::DealVideoPacket()
{
    ret = avcodec_send_packet(deVideoCodecCtx, dePacket);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(deVideoCodecCtx, deVideoFrame);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return;
            av_log_error("Error during decoding");
            return;
        }
        //av_log_info("receive a devideo frame\n");
        DealDeVideoFrame();
        av_frame_unref(deVideoFrame);
        if (ret < 0)
            return;
    }
}
void VideoFileContext::DealDeVideoFrame()
{
    if (swsCtx != nullptr)
        RescaleDevideoFrame();
    EncodeVideoFrame();
}
bool VideoFileContext::VariableCheck(const char *srcFilePath)
{
    av_log_info("video file context variable check start\n");
    // todo 更严格的控制
    if (FileContextBase::ret < 0 || VideoContext::ret < 0)
    {
        av_log_error("construct error\n");
        return false;
    }
    av_log_info("parent class init over\n");
    ResetFormatContextByFileName(srcFilePath);
    av_log_info("format context reset over\n");
    if (dePacket == nullptr) {
        dePacket = AllocAVPacket();
        av_log_info("depacket is nullptr,alloc over");
    }
    AVStream *videoStream = inFmtCtx->streams[getVideoStreamIndex()];
    av_log_info("find the video stream\n");
    if (deVideoCodecCtx == nullptr)
    {
        deVideoCodecCtx = OpenDecodecContextByStream(videoStream);
        av_log_info("deCodecContext is nullptr,now open the deCodecContext\n");
    }
    av_log_info("devideo codec context check success\n");
    if (deVideoCodecCtx->pix_fmt == AV_PIX_FMT_NONE) {
        if (deVideoCodecCtx->codec_id == AV_CODEC_ID_H264) {
            deVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
            srcPixFormat = AV_PIX_FMT_YUV420P;
        }
        else {
            deVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
            srcPixFormat = AV_PIX_FMT_YUV420P;
        }
    }
    else srcPixFormat = deVideoCodecCtx->pix_fmt;
    
    deWidth = deVideoCodecCtx->width;
    deHeight = deVideoCodecCtx->height;
    av_log_info("video file context variable check end\n");
    return VideoContext::VariableCheck();
}
VideoFileContext::VideoFileContext() : FileContextBase(), VideoContext()
{
}
VideoFileContext::VideoFileContext(int fps, int width, int height) : FileContextBase(),
                                                                     VideoContext(DEFAULTPIXFORMAT, DEFAULTVIDEOCODECID, fps, 0, 0, width, height)
{
}
VideoFileContext::VideoFileContext(int deWidth,int deHeight,int fps, int width, int height) : FileContextBase(),
VideoContext(DEFAULTPIXFORMAT, DEFAULTVIDEOCODECID, fps, deWidth, deHeight, width, height)
{
}
VideoFileContext::VideoFileContext(AVPixelFormat srcPixelFormat,AVCodecID dstCodecId,int fps, int width, int height) : FileContextBase(),
VideoContext(srcPixelFormat, dstCodecId, fps, 0, 0, width, height)
{
}
VideoFileContext::VideoFileContext(AVPixelFormat srcPixelFormat, AVCodecID dstCodecId, int deWidth,int deHeight,int fps, int width, int height) : FileContextBase(),
VideoContext(srcPixelFormat, dstCodecId, fps, deWidth,deHeight, width, height),deVideoCodecCtx(nullptr)
{
    av_log_info("VideoFileContextxt base init over\n");
    if (ret >= 0) {
        av_log_info("VideoFileContext init success\n");
    }
    else {
        av_log_error("VideoFileContext init failed\n");
    }
}
bool VideoFileContext::ExtractVideoToFile(const char *srcFilePath, const char *dstFilePath)
{
    if (VariableCheck(srcFilePath) == false)
        return false;
    WriteVideoPreparition(dstFilePath);
    DecodePakcet();
    WriteVideoTailer();
    return VideoContext::ret >= 0; // todo:使用getret，audio同理
}
VideoFileContext::~VideoFileContext()
{
    if(deVideoCodecCtx!=nullptr)
        avcodec_free_context(&deVideoCodecCtx);
}

int VideoFileContext::GetRet() const
{
    if (FileContextBase::ret < 0)return FileContextBase::ret;
    if (VideoContext::ret < 0)return VideoContext::ret;
    return ret;
}
