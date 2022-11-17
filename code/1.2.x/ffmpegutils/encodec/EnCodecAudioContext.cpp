#include "EnCodecAudioContext.h"

void EnCodecAudioContext::ReAllocFrame(int dstNbSamples)
{
    int format = frame->format;
    AVChannelLayout layout = frame->ch_layout;
    int sampleRate = frame->sample_rate;
    av_frame_free(&frame);
    frame = AllocAVFrame();
    frame->sample_rate = sampleRate;
    frame->format = format;
    av_channel_layout_copy(&frame->ch_layout, &layout);
    frame->nb_samples = dstNbSamples;
    // alloc buffer
    if ((ret = av_frame_get_buffer(frame, 0)) < 0)
    {
        av_log_error("frame get buffer is failed");
        return;
    }
    if ((ret = av_frame_make_writable(frame)) < 0)
    {
        av_log_error("frame is not writeable");
        return;
    }
    maxNbSamples = dstNbSamples;
}

bool EnCodecAudioContext::EncodeFrame(OutFormatContext& outFmtCont, AVStream* outStream)
{
    return EncodeFrame(outFmtCont, outStream, frame);
}

bool EnCodecAudioContext::EncodeFrame(OutFormatContext& outFmtCont, AVStream* outStream, AVFrame* enFrame)
{
    if (enFrame != nullptr)
    {
        enFrame->pts = pts;
        pts += enFrame->nb_samples;
    }
    AVFormatContext* fmtCont = outFmtCont.GetFormatContext();
    ret = avcodec_send_frame(codecCont, enFrame);
    if (ret < 0) return false;
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCont, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_log_pframe("eagain averror_eof %d,maybe encodecontext buffer has no enough frame data\n", ret);
            return false;
        }
        else if (ret < 0)
        {
            av_log_error("Error encoding audio frame\n");
            return false;
        }
        av_packet_rescale_ts(packet, codecCont->time_base, outStream->time_base);
        packet->stream_index = outStream->index;
        av_interleaved_write_frame(fmtCont, packet);
        //av_log_pframe("write a packet to out formatcontext success\n");
        av_packet_unref(packet);
        if (ret < 0)
            return false;
    }
    return true;
}

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
    if (dstNbSamples == 0) return frame;
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


EnCodecAudioContext::EnCodecAudioContext() :EnCodecContext(),maxNbSamples(0)
{
    
}

EnCodecAudioContext::EnCodecAudioContext(AVCodecID codecId):maxNbSamples(0)
{
#ifdef WINDOWS
    codecCont = CodecConfigManager::GetAudioEncoder(AudioEncodeName::LIBMP3MF);
#endif // WINDOWS
#ifdef ANDROID
    codecCont = CodecConfigManager::GetAudioEncoder(AudioEncodeName::LIBMP3LAME);
#endif // ANDROID


    if (codecCont == nullptr) {
        av_log_error("error when open encodec context,audio codeccontext initialize failed\n");
        goto end;
    }
    frame = InitAudioFrame(codecCont,maxNbSamples);
    if (frame == nullptr) {
        av_log_error("error when allocing frame,audio codeccontext initialize failed\n");
        goto end;
    }
    packet = AllocAVPacket();
    if (packet == nullptr) {
        av_log_error("error when allocing packet,audio codeccontext initialize failed\n");
        goto end;
    }
    ret = 0;
    return;
end:
    ret = -1;
    return;
}

bool EnCodecAudioContext::FlushBuffer(OutFormatContext& outFmtCont, AVStream* outStream)
{
    AVFrame* tempFrame = frame;
    frame = nullptr;
    bool result = EncodeFrame(outFmtCont, outStream);
    frame = tempFrame;
    return result;
}

int EnCodecAudioContext::GetNbSamplesOfFrameBuffer() const
{
    return maxNbSamples;
}

AVFrame* EnCodecAudioContext::GetEncodecFrame() const
{
    return frame;
}


EnCodecAudioContext::~EnCodecAudioContext()
{
    av_log_info("%s start", __FUNCTION__);
    av_frame_free(&frame);
    av_packet_free(&packet);
    av_log_info("%s end", __FUNCTION__);
}
