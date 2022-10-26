#include "VideoContext.h"

VideoContext::VideoContext() : enVideoCodecCtx(nullptr), outVideoFmtCtx(nullptr),
                               deVideoFrame(nullptr), enVideoFrame(nullptr), enVideoPacket(nullptr),
                               swsCtx(nullptr), pts(0), ret(0), deWidth(0), deHeight(0),
                               srcPixFormat(DEFAULTPIXFORMAT), outVideoStream(nullptr)
{
}
AVCodecContext *VideoContext::OpenVideoEncodecContext(AVCodecID enCodecid, int fps, int width, int height,float bitRatePercent)
{
    AVCodecContext* context = AllocEncodecContext(enCodecid);

    if (enCodecid == AV_CODEC_ID_H264)
    {
        context->pix_fmt = AV_PIX_FMT_YUV420P;
    }
    else if (enCodecid == AV_CODEC_ID_GIF) {
        context->pix_fmt = AV_PIX_FMT_PAL8;
    }
    else {
        context->pix_fmt = AV_PIX_FMT_RGBA;
    }
    context->codec_id = enCodecid;
    context->codec_type = AVMEDIA_TYPE_VIDEO;
    context->width = width;
    context->height = height;
    //context->rc_buffer_size = (int)context->bit_rate;
    context->framerate.num = fps;
    context->framerate.den = 1;
    context->time_base.num = 1;
    context->time_base.den = fps;
    context->thread_count = 4;
    context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    int64_t crf = (1 - bitRatePercent) * DEFAULTCRFMAXVALUE;
    if (crf<0 || crf>DEFAULTCRFMAXVALUE) crf = 18;
    av_opt_set_int(context->priv_data, "crf", crf, 0);
    av_opt_set(context->priv_data, "preset", "slow", 0);
    const AVCodec* codec = avcodec_find_encoder(enCodecid);
    av_log_info("is opening video codec context\n");
    av_log_info("bit rate percent from unity:%f,target crf:%ld\n", bitRatePercent, crf);
    av_log_info("context bitrate:%ld\n", context->bit_rate);
    ret = avcodec_open2(context, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec context\n");
        //return nullptr;
    }
    av_log_info("open video codec context end\n");
    return context;
}
SwsContext *VideoContext::InitSwsContext(AVCodecContext *deCodecCtx, AVCodecContext *enCodecCtx)
{
    SwsContext *context = nullptr;
    context = sws_getCachedContext(context, deCodecCtx->width, deCodecCtx->height, deCodecCtx->pix_fmt,
                                   enCodecCtx->width, enCodecCtx->height, enCodecCtx->pix_fmt,
                                   SWS_BICUBIC, nullptr, nullptr, nullptr);
    return context;
}
SwsContext *VideoContext::InitSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight, AVCodecContext *enCodecCtx)
{
    SwsContext *context = nullptr;
    context = sws_getCachedContext(context, deWidth, deHeight, srcPixFormat,
                                   enCodecCtx->width, enCodecCtx->height, enCodecCtx->pix_fmt,
                                   SWS_BICUBIC, nullptr, nullptr, nullptr);
    return context;
}
AVFrame *VideoContext::CreateVideoAVFrame(AVCodecContext *codecContext)
{
    AVFrame *frame = AllocAVFrame();
    frame->format = codecContext->pix_fmt;
    frame->width = codecContext->width;
    frame->height = codecContext->height;
    ret = av_frame_get_buffer(frame, 0); // error prone
    if (ret < 0)
    {
        av_log_error("could not allocate the frame data buffer\n");
    }
    return frame;
}
int VideoContext::GetPixFormatChannelNumber() const
{
    // add more pix format in future
    if (srcPixFormat == AV_PIX_FMT_RGBA)
        return 4;
    else
        return 3;
}
void VideoContext::RescaleDevideoFrame()
{
    if (swsCtx == nullptr)
        return;
    ret = sws_scale(swsCtx, deVideoFrame->data, deVideoFrame->linesize, 0,
                    deHeight == 0 ? deVideoFrame->height : deHeight, enVideoFrame->data, enVideoFrame->linesize);
    //av_log_info("sws scale result:%d\n", ret);
    if (ret <= 0)
    {
        av_log_error("sws scale the video frame failed\n");
    }
}
void VideoContext::EncodeVideoFrame()
{
    if (enVideoFrame != nullptr)
    {
        enVideoFrame->pts = pts;
        pts += 1;
    }
    ret = avcodec_send_frame(enVideoCodecCtx, enVideoFrame);
    av_log_info("send frame ret:%d", ret);
    if (ret < 0)
    {
        av_log_error("send frame to enVideoCodecContext error\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enVideoCodecCtx, enVideoPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_log_error("eagain averror_eof %d\n", ret);
            return;
        }

        if (ret < 0)
        {
            av_log_error("enVideoCodecContext receive packet error\n");
            return;
        }
        av_packet_rescale_ts(enVideoPacket, enVideoCodecCtx->time_base, outVideoStream->time_base);
        enVideoPacket->stream_index = outVideoStream->index;
        av_interleaved_write_frame(outVideoFmtCtx, enVideoPacket);
        av_packet_unref(enVideoPacket);
        if (ret < 0)
            return;
    }
}
bool VideoContext::VariableCheck()
{
    if (ret < 0)
    {
        av_log_error("construct error\n");
        return false;
    }
    if (deVideoFrame == nullptr)
    {
        deVideoFrame = AllocAVFrame();
        deVideoFrame->format = srcPixFormat;
        deVideoFrame->width = deWidth;
        deVideoFrame->height = deHeight;
        deVideoFrame->linesize[0] = deWidth * GetPixFormatChannelNumber();
        av_log_info("devideo frame is nullptr,allocting avframe succeed,frame linsize[0]=%d\n", deVideoFrame->linesize[0]);
        if (deWidth == 0)
        {
            av_log_info("warning:devideo frame width is 0\n");
        }
    }
    if (enVideoCodecCtx != nullptr)
    {
        if (deHeight != 0 && deWidth != 0 && (srcPixFormat != enVideoCodecCtx->pix_fmt || deWidth != enVideoCodecCtx->width || deHeight != enVideoCodecCtx->height)&&swsCtx==nullptr)
        {
            sws_freeContext(swsCtx);
            swsCtx = InitSwsContext(srcPixFormat, deWidth, deHeight, enVideoCodecCtx);
            av_log_info("swscontext is nullptr,alloced swscontext\n");
        }
        /*else
        {
            sws_freeContext(swsCtx);
            swsCtx = nullptr;
        }*/
        if (swsCtx == nullptr)
        {
            //不需要转码
            av_frame_free(&enVideoFrame);
            enVideoFrame = deVideoFrame;
        }
        if (enVideoPacket == nullptr)
        {
            enVideoPacket = AllocAVPacket();
        }
    }
    else
    {
        av_log_info("warning:envideo codeccontext is nullptr\n");
    }
    return true;
}
VideoContext::VideoContext(AVCodecID codecId, int fps, float bitRatePercent,int width, int height) : VideoContext(DEFAULTPIXFORMAT, codecId, fps, bitRatePercent,width, height)
{
}
VideoContext::VideoContext(AVPixelFormat dePixFormat, AVCodecID codecId,
                           int fps, float bitRatePercent,int width, int height) : VideoContext(dePixFormat, codecId, fps, bitRatePercent,width, height, width, height)
{
}
VideoContext::VideoContext(AVPixelFormat dePixFormat, AVCodecID codecId,
                           int fps, float bitRatePercent,int deWidth, int deHeight,
                           int enWidth, int enHeight) : enVideoCodecCtx(OpenVideoEncodecContext(codecId, fps, enWidth, enHeight, bitRatePercent)),
                                                        swsCtx(nullptr), enVideoFrame(CreateVideoAVFrame(enVideoCodecCtx)),
                                                        enVideoPacket(AllocAVPacket()), deVideoFrame(nullptr), pts(0), ret(0),
                                                        outVideoFmtCtx(nullptr), outVideoStream(nullptr), srcPixFormat(dePixFormat)
{
    this->deWidth = deWidth;
    this->deHeight = deHeight;
    if (deWidth != 0 && deHeight != 0 && (deWidth != enWidth || deHeight != enHeight || dePixFormat != enVideoCodecCtx->pix_fmt))
    {
        swsCtx = InitSwsContext(dePixFormat, deWidth, deHeight, enVideoCodecCtx);
    }
}
// function region of writing to file
void VideoContext::WriteVideoPreparition(const char *dstFilePath)
{
    if (deVideoFrame == nullptr)
    {
        deVideoFrame = AllocAVFrame();
        deVideoFrame->linesize[0] = deWidth * GetPixFormatChannelNumber();
        av_log_info("devideo frame is nullptr,allocting avframe succeed\n");
    }
    outVideoFmtCtx = AllocOutFormatContext(dstFilePath);
    if (outVideoFmtCtx == nullptr)
    {
        ret = -1;
        av_log_error("alloc outformat context failed\n");
        return;
    }
    AddNewStreamToFormat(outVideoFmtCtx, enVideoCodecCtx);
    outVideoStream = outVideoFmtCtx->streams[outVideoFmtCtx->nb_streams - 1];
    // open target file stream
    ret = avio_open(&outVideoFmtCtx->pb, dstFilePath, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        av_log_error("avio open failed\n");
        return;
    }
    // write file header information
    ret = avformat_write_header(outVideoFmtCtx, nullptr);
    if (ret < 0)
    {
        av_log_error("write header information failed\n");
        return;
    }
}
void VideoContext::WriteVideoToFile(AVFrame *deVideoFrame)
{
    ret = av_frame_copy_props(this->deVideoFrame, deVideoFrame);
    for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i)
    {
        this->deVideoFrame->data[i] = deVideoFrame->data[i];
        this->deVideoFrame->linesize[i] = deVideoFrame->linesize[i];
    }
    RescaleDevideoFrame();
    EncodeVideoFrame();
}
void VideoContext::WriteVideoToFile(void *data, int length)
{
    deVideoFrame->data[0] = (uint8_t *)data;
    RescaleDevideoFrame();
    EncodeVideoFrame();
}
void VideoContext::WriteVideoToFile()
{
    RescaleDevideoFrame();
    EncodeVideoFrame();
}
void VideoContext::WriteVideoTailer()
{
    ret = av_write_trailer(outVideoFmtCtx);
    if (ret < 0)
    {
        av_log_error("write trailer information failed\n");
        return;
    }
    ret = avio_close(outVideoFmtCtx->pb);
    if (ret < 0)
    {
        av_log_error("close outfmt context failed\n");
        return;
    }
    avformat_free_context(outVideoFmtCtx);
    outVideoFmtCtx = nullptr;
    outVideoStream = nullptr;

    av_frame_free(&deVideoFrame);
    deVideoFrame = nullptr;

    pts = 0;
}
int VideoContext::Flip(unsigned char* src)
{
    unsigned char* tempSrc = NULL;
    int srcW = enVideoCodecCtx->width;
    int srcH = enVideoCodecCtx->height;
    int channel = 4;
    int mSize = srcW * srcH * sizeof(char) * channel;
    int i = 0;
    int j = 0;
    int k = 0;
    int desW = 0;
    int desH = 0;

    desW = srcW;
    desH = srcH;

    tempSrc = (unsigned char*)malloc(sizeof(char) * srcW * srcH * channel);
    memcpy(tempSrc, src, mSize);

    for (i = 0; i < desH; i++)
    {
        for (j = 0; j < desW; j++)
        {
            for (k = 0; k < channel; k++)
            {
                src[(i * desW + j) * channel + k] = tempSrc[((srcH - 1 - i) * srcW + j) * channel + k];
            }
        }
    }

    free(tempSrc);
    return 0;
}
void VideoContext::FlushEnVideoCodecBuffer()
{
    av_frame_free(&enVideoFrame);
    enVideoFrame = nullptr;
    EncodeVideoFrame();
}
VideoContext::~VideoContext()
{
    av_packet_free(&enVideoPacket);
    av_frame_free(&enVideoFrame);
    if (swsCtx != nullptr)
        sws_freeContext(swsCtx);
    if (outVideoFmtCtx != nullptr)
        avformat_free_context(outVideoFmtCtx);
}