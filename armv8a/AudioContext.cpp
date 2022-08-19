#include "AudioContext.h"

AVCodecContext* AudioContext::OpenAudioEncodecContext(AVCodecID codecId)
{
    AVCodecContext* codecContext = AllocEnCodecContext(codecId);
    SetAudioEssentialParameters(codecContext, codecId);
    const AVCodec* codec = avcodec_find_encoder(codecId);
    codecContext->sample_fmt = codec->sample_fmts[0];
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec\n");
    }
    return codecContext;
}

void AudioContext::InitAudioFrame(AVFrame*& frame, AVCodecContext* codecCtx, int dstNbSamples)
{
    av_frame_free(&frame);
    frame = AllocAVFrame();
    frame->sample_rate = codecCtx->sample_rate;
    frame->format = codecCtx->sample_fmt;
    av_channel_layout_copy(&frame->ch_layout, &codecCtx->ch_layout);
    frame->nb_samples = dstNbSamples;
    //����buffer
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
}
void AudioContext::ResampleDeAudioFrame()
{
    int64_t delay = swr_get_delay(swrCtx, deAudioFrame->sample_rate);
    int64_t dstNbSamples = av_rescale_rnd(delay + deAudioFrame->nb_samples, enAudioCodecCtx->sample_rate,
        deAudioFrame->sample_rate, AV_ROUND_UP);
    if (dstNbSamples > maxDstNbSamples)
    {
        InitAudioFrame(enAudioFrame, enAudioCodecCtx, dstNbSamples);
        maxDstNbSamples = dstNbSamples;
    }
    ret = swr_convert(swrCtx, enAudioFrame->data, dstNbSamples,
        const_cast<const uint8_t**>(deAudioFrame->data), deAudioFrame->nb_samples);
    if (ret < 0)
    {
        av_log_error("resample is failed\n");
        return;
    }
}
void AudioContext::EncodeAudioFrame()
{
    ret = avcodec_send_frame(enAudioCodecCtx, enAudioFrame);
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enAudioCodecCtx, enAudioPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            av_log_error("Error encoding audio frame");
            return;
        }
        fwrite(enAudioPacket->data, 1, enAudioPacket->size, dstFilePtr);
        av_packet_unref(enAudioPacket);
    }
}
void AudioContext::DealAudioPacket(AVPacket* packet)
{
    ret = avcodec_send_packet(deAudioCodecCtx, packet);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(deAudioCodecCtx, deAudioFrame);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return;

            av_log_error("Error during decoding");
            return;
        }
        ResampleDeAudioFrame();
        EncodeAudioFrame();
        av_frame_unref(deAudioFrame);
        if (ret < 0)
            return;
    }
}
SwrContext* AudioContext::AllocSwrContext(AVCodecContext* deCodecCtx, AVCodecContext* enCodecCtx)
{
    SwrContext* swrCtx = swr_alloc();
    if (!swrCtx)
    {
        ret = AVERROR(ENOMEM);
        av_log_error("could not allocate resampler context");
        return swrCtx;
    }
    // set swrcontext essential parameters
    av_opt_set_chlayout(swrCtx, "in_chlayout", &(deCodecCtx->ch_layout), 0);
    av_opt_set_int(swrCtx, "in_sample_rate", deCodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", deCodecCtx->sample_fmt, 0);
    av_opt_set_chlayout(swrCtx, "out_chlayout", &(enCodecCtx->ch_layout), 0);
    av_opt_set_int(swrCtx, "out_sample_rate", enCodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", enCodecCtx->sample_fmt, 0);

    ret = swr_init(swrCtx);
    if (ret < 0)
    {
        av_log_error("failed to initialize the resampling context\n");
    }
    return swrCtx;
}
void AudioContext::SetAudioEssentialParameters(AVCodecContext*& codecCtx, AVCodecID codecId)
{
    switch (codecId)
    {
    case AV_CODEC_ID_MP3:
    {
        codecCtx->bit_rate = 192000;
        codecCtx->sample_rate = 48000;
        AVChannelLayout layout = { AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO} };
        av_channel_layout_copy(&codecCtx->ch_layout, &layout);
    }
    break;
    default:
        av_log_error("audio type now is not supported\n");
        ret = -1;
        break;
    }
}
AudioContext::AudioContext(AVCodecID dstCodecID) : ContextBase(), dstFilePtr(nullptr), swrCtx(nullptr),
deAudioFrame(nullptr), deAudioCodecCtx(nullptr),
enCodecId(dstCodecID), maxDstNbSamples(0)
{
    enAudioCodecCtx = OpenAudioEncodecContext(enCodecId);
    enAudioFrame = AllocAVFrame();
    enAudioPacket = AllocAVPacket();
}
bool AudioContext::ExtractAudioToFile(const char* srcFilePath, const char* dstFilePath)
{
    if (ret < 0)
    {
        av_log_error("construct error\n");
        return false;
    }
    // if (enCodecId != AV_CODEC_ID_MP3)
    // {
    //     av_log_error("enCodecId!=AV_CODEC_ID_MP3,please reset enCodecId\n");
    //     return false;
    // }
    // set contextbase members
    ReSetAVFormatContext(srcFilePath);
    if (dePacket == nullptr)
        dePacket = AllocAVPacket();
    // set decodeccontext
    int audioStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    AVStream* audioStream = fmtCtx->streams[audioStreamIndex];
    AVCodecID deCodecId = audioStream->codecpar->codec_id;
    if (deAudioCodecCtx == nullptr)
    {
        deAudioCodecCtx = OpenDecodecContextByStreamPar(audioStream);
        av_log_info("deCodecContext is nullptr,now open the deCodecContext\n");
    }
    else
    {
        if (deCodecId != deAudioCodecCtx->codec_id)
        {
            // reset decodeccontxt
            avcodec_free_context(&deAudioCodecCtx);
            deAudioCodecCtx = OpenDecodecContextByStreamPar(audioStream);
            av_log_info("deCodecContext need to change,reopen the deCodecContext\n");
        }
    }
    // set swrcontext
    if (swrCtx == nullptr)
    {
        swrCtx = AllocSwrContext(deAudioCodecCtx, enAudioCodecCtx);
        av_log_info("swrcontext is nullptr,alloced swrcontext\n");
    }
    else
    {
        // reset swrcontext
        // todo: is need reset swrcontext
        swr_free(&swrCtx);
        swrCtx = AllocSwrContext(deAudioCodecCtx, enAudioCodecCtx);
        av_log_info("swrcontext need to change,reset swrcontext\n");
    }
    maxDstNbSamples = av_rescale_rnd(deAudioCodecCtx->frame_size, deAudioCodecCtx->sample_rate,
        enAudioCodecCtx->sample_rate, AV_ROUND_UP);
    deAudioFrame = AllocAVFrame();
    // start decode and write to dst file
    dstFilePtr = fopen(dstFilePath, "wb");
    if (dstFilePtr == nullptr)
    {
        ret = -1;
        av_log_error("%s can not open\n", dstFilePath);
        return false;
    }
    DecodePacket(fmtCtx);
    fclose(dstFilePtr);
    av_frame_free(&deAudioFrame);
    return ret >= 0;
}
void AudioContext::ResetAudioCodecId(AVCodecID codecId)
{
    if (codecId == enCodecId)
    {
        av_log_info("AVCodecID is not change\n");
        return;
    }
    enCodecId = codecId;
    avcodec_free_context(&enAudioCodecCtx);
    enAudioCodecCtx = OpenAudioEncodecContext(enCodecId);
}
AudioContext::~AudioContext()
{
    swr_free(&swrCtx);
    av_frame_free(&enAudioFrame);
    av_packet_free(&enAudioPacket);
    avcodec_free_context(&deAudioCodecCtx);
    avcodec_free_context(&enAudioCodecCtx);
}