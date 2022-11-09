#include "EnCodecVideoContext.h"

const std::string EnCodecVideoContext::presetLevels[9] = {"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow"};
AVCodecContext *EnCodecVideoContext::OpenEncodecContext(AVCodecID enCodecid, int width, int height, int fps, float bitRatePercent, int crfMin, int crfMax, int presetLevel)
{
    AVCodecContext *context = AllocEncodecContext(enCodecid);
    if (context == nullptr)
    {
        ret = -1;
        return nullptr;
    }
    if (enCodecid == AV_CODEC_ID_H264)
    {
        context->pix_fmt = AV_PIX_FMT_YUV420P;
    }
    else if (enCodecid == AV_CODEC_ID_GIF)
    {
        context->pix_fmt = AV_PIX_FMT_PAL8;
    }
    else
    {
        context->pix_fmt = AV_PIX_FMT_RGBA;
    }
    context->codec_id = enCodecid;
    context->codec_type = AVMEDIA_TYPE_VIDEO;
    context->width = width;
    context->height = height;
    // context->rc_buffer_size = (int)context->bit_rate;
    context->framerate.num = fps;
    context->framerate.den = 1;
    context->time_base.num = 1;
    context->time_base.den = fps;
    context->thread_count = 4;
    context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    int64_t crf = crfMin + (1 - bitRatePercent) * (crfMax - crfMin);
    if (crf < 0 || crf > DEFAULTCRFMAXVALUE)
        crf = 23;
    av_log_info("crf interval:[%d,%d]", crfMin, crfMax);
    av_log_info("bit rate percent from unity:%f,target crf:%ld\n", bitRatePercent, crf);
    av_opt_set_int(context->priv_data, "crf", crf, 0);
    // av_opt_set_int(context->priv_data, "qp", 18, 0);
    if (presetLevel < 0 || presetLevel > 8)
        presetLevel = 0;
    // av_opt_set(context->priv_data, "preset", presetLevels[presetLevel].c_str(), 0);
    av_log_info("presetlevel from unity:%d,the string is %s\n", presetLevel, presetLevels[presetLevel].c_str());
    const AVCodec *codec = avcodec_find_encoder(enCodecid);
    av_log_info("is opening video codec context\n");
    av_log_info("bit rate percent from unity:%f,target crf:%ld\n", bitRatePercent, crf);
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

AVFrame *EnCodecVideoContext::CreateVideoFrame(const AVCodecContext *codeCont)
{
    AVFrame *frame = AllocAVFrame();
    if (frame == nullptr)
        return nullptr;
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

EnCodecVideoContext::EnCodecVideoContext() : EnCodecContext()
{
}

EnCodecVideoContext::EnCodecVideoContext(AVCodecID encodeId, int width, int height, int fps, float bitRatePercent, int crfMin, int crfMax, int presetLevel)
{
    codecCont = OpenEncodecContext(encodeId, width, height, fps, bitRatePercent, crfMin, crfMax, presetLevel);
    av_log_info("video encoder name:%s", codecCont->codec->name);
    if (codecCont == nullptr)
    {
        av_log_error("error when open encodec context,video codeccontext initialize failed\n");
        goto end;
    }
    av_log_info("encodec context open success\n");
    frame = CreateVideoFrame(codecCont);
    if (frame == nullptr)
    {
        av_log_error("error when allocing frame,video codeccontext initialize failed\n");
        goto end;
    }
    av_log_info("video frame create success\n");
    packet = AllocAVPacket();
    if (packet == nullptr)
    {
        av_log_error("error when allocing packet,video codeccontext initialize failed\n");
        goto end;
    }
    av_log_info("packet alloc success\n");
    ret = 0;
    return;
end:
    ret = -1;
    return;
}

EnCodecVideoContext::EnCodecVideoContext(EncodeName encodeName, int width, int height, int fps,
                                         float bitRatePercent, int crfMin, int crfMax,
                                         int presetLevel) : EnCodecContext(), inFrameCount(0)
{
    codecCont = CodecConfigManager::GetVideoEncoder(encodeName, width, height, fps, bitRatePercent, crfMin, crfMax, (PresetLevel)presetLevel);
    // codecCont = OpenEncodecContext(codecId,width,height,fps,bitRatePercent,crfMin,crfMax,presetLevel);
    av_log_info("video encoder name:%s", codecCont->codec->name);
    if (codecCont == nullptr)
    {
        av_log_error("error when open encodec context,video codeccontext initialize failed\n");
        goto end;
    }
    av_log_info("encodec context open success\n");
    frame = CreateVideoFrame(codecCont);
    if (frame == nullptr)
    {
        av_log_error("error when allocing frame,video codeccontext initialize failed\n");
        goto end;
    }
    av_log_info("video frame create success\n");
    packet = AllocAVPacket();
    if (packet == nullptr)
    {
        av_log_error("error when allocing packet,video codeccontext initialize failed\n");
        goto end;
    }
    av_log_info("packet alloc success\n");
    ret = 0;
    return;
end:
    ret = -1;
    return;
}

bool EnCodecVideoContext::EncodeFrame(OutFormatContext &outFmtCont, AVStream *outStream)
{
    return EncodeFrame(outFmtCont, outStream, frame);
}

bool EnCodecVideoContext::EncodeFrame(OutFormatContext &outFmtCont, AVStream *outStream, AVFrame *enFrame)
{
    if (enFrame != nullptr)
    {
        enFrame->pts = pts;
        pts += 1;
    }
    ret = avcodec_send_frame(codecCont, enFrame);
    if (ret < 0)
    {
        av_log_error("send frame to enVideoCodecContext error,ret:%d\n", ret);
        return false;
    }
    ++inFrameCount;
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCont, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            // av_log_pframe("eagain averror_eof %d,maybe encodecontext buffer has no enough frame data\n", ret);
            return false;
        }

        if (ret < 0)
        {
            av_log_error("enVideoCodecContext receive packet error\n");
            return false;
        }
        av_packet_rescale_ts(packet, codecCont->time_base, outStream->time_base);
        packet->stream_index = outStream->index;
        av_interleaved_write_frame(outFmtCont.GetFormatContext(), packet);
        av_packet_unref(packet);
        if (ret < 0)
            return false;
    }
    return true;
}

bool EnCodecVideoContext::FlushBuffer(OutFormatContext &outFmtCont, AVStream *outStream)
{
    AVFrame *tempFrame = frame;
    frame = nullptr;
    int result = EncodeFrame(outFmtCont, outStream);
    frame = tempFrame;
    av_log_info("encode frame count:%d\n", inFrameCount);
    return result;
}

AVFrame *EnCodecVideoContext::GetEncodecFrame() const
{
    return frame;
}

EnCodecVideoContext::~EnCodecVideoContext()
{
    av_frame_free(&frame);
    av_packet_free(&packet);
}
