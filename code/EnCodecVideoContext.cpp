#include "EnCodecVideoContext.h"

AVCodecContext* EnCodecVideoContext::OpenEncodecContext(AVCodecID enCodecid,int width,int height,int fps,float bitRatePercent)
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
    /*context->thread_count = 4;
    context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;*/
    /*int64_t crf =  (1- bitRatePercent)* DEFAULTCRFMAXVALUE;
    if (crf<0 || crf>DEFAULTCRFMAXVALUE) crf = 18;
    av_opt_set_int(context->priv_data, "crf", crf, 0);*/
    //av_opt_set_int(context->priv_data, "qp", 18, 0);
    //av_opt_set(context->priv_data, "preset", "veryslow", 0);
    const AVCodec* codec = avcodec_find_encoder(enCodecid);
    av_log_info("is opening video codec context\n");
    //av_log_info("bit rate percent from unity:%f,target crf:%ld\n", bitRatePercent, crf);
    av_log_info("context bitrate:%ld\n", context->bit_rate);
    ret = avcodec_open2(context, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec context\n");
        return nullptr;
    }
    av_log_info("open video codec context end\n");
    return context;
}

AVFrame* EnCodecVideoContext::CreateVideoFrame(const AVCodecContext* codeCont)
{
    AVFrame* frame = AllocAVFrame();
    if (frame == nullptr)return nullptr;
    frame->format = codeCont->pix_fmt;
    frame->width = codeCont->width;
    frame->height = codeCont->height;
    ret = av_frame_get_buffer(frame, 0); // error prone
    if (ret < 0)
    {
        av_log_error("could not allocate the frame data buffer\n");
        av_frame_free(&frame);
        frame = nullptr;
        return nullptr;
    }
    return frame;
}

EnCodecVideoContext::EnCodecVideoContext() :EnCodecContext(), frame(nullptr), pts(0), packet(nullptr)
{
}

EnCodecVideoContext::EnCodecVideoContext(AVCodecID codecId, int width, int height, int fps, float bitRatePercent):EnCodecContext(),pts(0)
{
    codecCont = OpenEncodecContext(codecId,width,height,fps,bitRatePercent);
    if (codecCont == nullptr) {
        av_log_error("error when open encodec context,video codeccontext initialize failed\n");
        goto end;
    }
    frame = CreateVideoFrame(codecCont);
    if (frame == nullptr) {
        av_log_error("error when allocing frame,video codeccontext initialize failed\n");
        goto end;
    }
    packet = AllocAVPacket();
    if (packet == nullptr) {
        av_log_error("error when allocing packet,video codeccontext initialize failed\n");
        goto end;
    }
    return;
end:
    ret = -1;
    return;
}

bool EnCodecVideoContext::EncodeVideoFrame(AVFormatContext* fmtCont, AVStream* outStream)
{
    if (frame != nullptr)
    {
        frame->pts = pts;
        pts += 1;
    }
    ret = avcodec_send_frame(codecCont, frame);
    if (ret < 0)
    {
        av_log_error("send frame to enVideoCodecContext error,ret:%d\n",ret);
        return false;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCont, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_log_error("eagain averror_eof %d\n", ret);
            return false;
        }

        if (ret < 0)
        {
            av_log_error("enVideoCodecContext receive packet error\n");
            return false;
        }
        av_packet_rescale_ts(packet, codecCont->time_base, outStream->time_base);
        packet->stream_index = outStream->index;
        av_interleaved_write_frame(fmtCont, packet);
        av_packet_unref(packet);
        if (ret < 0)
            return false;
    }
    return true;
}

bool EnCodecVideoContext::EncodeVideoFrame(AVFormatContext* fmtCont, AVStream* outStream, AVFrame* videoFrame)
{
    if (videoFrame != nullptr)
    {
        videoFrame->pts = pts;
        pts += 1;
    }
    ret = avcodec_send_frame(codecCont, videoFrame);
    if (ret < 0)
    {
        av_log_error("send frame to enVideoCodecContext error,ret:%d\n", ret);
        return false;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCont, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_log_error("eagain averror_eof %d\n", ret);
            return false;
        }

        if (ret < 0)
        {
            av_log_error("enVideoCodecContext receive packet error\n");
            return false;
        }
        av_packet_rescale_ts(packet, codecCont->time_base, outStream->time_base);
        packet->stream_index = outStream->index;
        av_interleaved_write_frame(fmtCont, packet);
        av_packet_unref(packet);
        if (ret < 0)
            return false;
    }
    return true;
}

bool EnCodecVideoContext::FlushBuffer(AVFormatContext* fmtCont, AVStream* outStream)
{
    AVFrame* tempFrame = frame;
    frame = nullptr;
    int result = EncodeVideoFrame(fmtCont, outStream);
    frame = tempFrame;
    return result;
}

AVFrame* EnCodecVideoContext::GetEncodecFrame() const
{
    return frame;
}

EnCodecVideoContext::~EnCodecVideoContext()
{
    av_frame_free(&frame);
    av_packet_free(&packet);
}
