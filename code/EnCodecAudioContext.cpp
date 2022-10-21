#include "EnCodecAudioContext.h"

AVCodecContext* EnCodecAudioContext::OpenEncodecContext(AVCodecID encodecid)
{
    AVCodecContext* codecContext = AllocEncodecContext(encodecid);
    AVChannelLayout layout;
    switch (encodecid)
    {
    case AV_CODEC_ID_MP3:
        codecContext->bit_rate = 320000;
        codecContext->sample_rate = 48000; // error prone
        layout = { AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO} };
        av_channel_layout_copy(&codecContext->ch_layout, &layout);
        break;
    case AV_CODEC_ID_AAC:
        codecContext->bit_rate = 137000;
        codecContext->sample_rate = 44100; // error prone
        layout = { AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO} };
        av_channel_layout_copy(&codecContext->ch_layout, &layout);
        break;
    default:
        break;
    }

    const AVCodec* codec = avcodec_find_encoder(encodecid);
    codecContext->sample_fmt = codec->sample_fmts[0];
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec\n");
        return nullptr;
    }
    return codecContext;
}

AVFrame* EnCodecAudioContext::InitAudioFrame(AVCodecContext* codeCont, int dstNbSamples)
{
    AVFrame* frame = AllocAVFrame();
    if (frame == nullptr) return nullptr;
    frame->sample_rate = codeCont->sample_rate;
    frame->format = codecCont->sample_fmt;
    av_channel_layout_copy(&frame->ch_layout, &(codeCont->ch_layout));
    frame->nb_samples = dstNbSamples;
    // alloc buffer
    if ((ret = av_frame_get_buffer(frame, 0)) < 0)
    {
        av_log_error("frame get buffer is failed");
        return nullptr;
    }
    if ((ret = av_frame_make_writable(frame)) < 0)
    {
        av_log_error("frame is not writeable");
        return nullptr;
    }
    return frame;
}


EnCodecAudioContext::EnCodecAudioContext() :EnCodecContext(),frame(nullptr),pts(0),packet(nullptr)
{

}

EnCodecAudioContext::EnCodecAudioContext(AVCodecID codecId):pts(0)
{
    codecCont = OpenEncodecContext(codecId);
    if (codecCont == nullptr) {
        av_log_error("error when open encodec context,audio codeccontext initialize failed\n");
        goto end;
    }
    frame = InitAudioFrame(codecCont,800);
    if (frame == nullptr) {
        av_log_error("error when allocing frame,audio codeccontext initialize failed\n");
        goto end;
    }
    packet = AllocAVPacket();
    if (packet == nullptr) {
        av_log_error("error when allocing packet,audio codeccontext initialize failed\n");
        goto end;
    }
end:
    ret = -1;
    return;
}

bool EnCodecAudioContext::EncodeAudioFrame(AVFormatContext* fmtCont,AVStream* outStream)
{
    if (frame != nullptr)
    {
        frame->pts = pts;
        pts += frame->nb_samples;
    }
    ret = avcodec_send_frame(codecCont, frame);
    if (ret < 0) return false;
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCont, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return false;
        else if (ret < 0)
        {
            av_log_error("Error encoding audio frame\n");
            return false;
        }
        av_packet_rescale_ts(packet, codecCont->time_base, outStream->time_base);
        packet->stream_index = outStream->index;
        av_interleaved_write_frame(fmtCont, packet);
        av_log_info("write a packet to out formatcontext success\n");
        av_packet_unref(packet);
        if (ret < 0)
            return false;
    }
    return true;
}

bool EnCodecAudioContext::FlushBuffer(AVFormatContext* fmtCont, AVStream* outStream)
{
    AVFrame* tempFrame = frame;
    frame = nullptr;
    bool result = EncodeAudioFrame(fmtCont,outStream);
    frame = tempFrame;
    return result;
}

AVFrame* EnCodecAudioContext::GetEncodecFrame() const
{
    return frame;
}

EnCodecAudioContext::~EnCodecAudioContext()
{
    av_frame_free(&frame);
    av_packet_free(&packet);
}
