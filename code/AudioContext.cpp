#include "AudioContext.h"

AudioContext::AudioContext() : enAudioCodecCtx(nullptr), outAudioFmtCtx(nullptr),
                               deAudioFrame(nullptr), enAudioFrame(nullptr),
                               enAudioPacket(nullptr), swrCtx(nullptr),
                               maxDstNbSamples(0), pts(0),
                               srcSampleRate(0), srcSampleFormat(), srcChlayout()
{
}
void AudioContext::ResampleDeAudioFrame()
{
    if (swrCtx == nullptr)return;
    int64_t delay = swr_get_delay(swrCtx, deAudioFrame->sample_rate);
    int dstNbSamples = av_rescale_rnd(delay + deAudioFrame->nb_samples, enAudioCodecCtx->sample_rate,
                                      deAudioFrame->sample_rate, AV_ROUND_UP);
    if (dstNbSamples > maxDstNbSamples)
    {
        InitEnAudioFrame(dstNbSamples);
        maxDstNbSamples = dstNbSamples;
    }
    ret = swr_convert(swrCtx, enAudioFrame->data, dstNbSamples,
                      const_cast<const uint8_t **>(deAudioFrame->data), deAudioFrame->nb_samples);
    enAudioFrame->nb_samples = ret; // error prone
    //av_log_info("resample success,EnAudio Frame sample number:%d", ret);
    if (ret < 0)
    {
        av_log_error("resample is failed\n");
        return;
    }
}
void AudioContext::EncodeAudioFrame()
{
    if (enAudioFrame != nullptr)
    {
        enAudioFrame->pts = pts;
        pts += enAudioFrame->nb_samples;
    }
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
        av_packet_rescale_ts(enAudioPacket, enAudioCodecCtx->time_base, outAudioStream->time_base);
        enAudioPacket->stream_index = outAudioStream->index;
        av_interleaved_write_frame(outAudioFmtCtx, enAudioPacket);
        av_log_info("write a packet to out formatcontext success\n");
        av_packet_unref(enAudioPacket);
        if (ret < 0)
            return;
    }
}
void AudioContext::InitEnAudioFrame(int dstNbSamples)
{
    av_frame_free(&enAudioFrame);
    enAudioFrame = AllocAVFrame();
    enAudioFrame->sample_rate = enAudioCodecCtx->sample_rate;
    enAudioFrame->format = enAudioCodecCtx->sample_fmt;
    av_channel_layout_copy(&enAudioFrame->ch_layout, &enAudioCodecCtx->ch_layout);
    enAudioFrame->nb_samples = dstNbSamples;
    // alloc buffer
    if ((ret = av_frame_get_buffer(enAudioFrame, 0)) < 0)
    {
        av_log_error("frame get buffer is failed");
        return;
    }
    if ((ret = av_frame_make_writable(enAudioFrame)) < 0)
    {
        av_log_error("frame is not writeable");
        return;
    }
}
AVCodecContext *AudioContext::OpenAudioEncodecContext(AVCodecID encodecid)
{
    AVCodecContext *codecContext = AllocEncodecContext(encodecid);
    AVChannelLayout layout;
    switch (encodecid)
    {
    case AV_CODEC_ID_MP3:
        codecContext->bit_rate = 320000;
        codecContext->sample_rate = 48000; // error prone
        layout = {AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO}};
        av_channel_layout_copy(&codecContext->ch_layout, &layout);
        break;
    case AV_CODEC_ID_AAC:
        codecContext->bit_rate = 137000;
        codecContext->sample_rate = 44100; // error prone
        layout = {AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO}};
        av_channel_layout_copy(&codecContext->ch_layout, &layout);
        break;
    default:
        break;
    }

    const AVCodec *codec = avcodec_find_encoder(encodecid);
    codecContext->sample_fmt = codec->sample_fmts[0];
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec\n");
        return nullptr;
    }
    return codecContext;
}
SwrContext *AudioContext::InitSwrContext(AVCodecContext *deCodecCtx, AVCodecContext *enCodecCtx)
{
    SwrContext *swrCtx = swr_alloc();
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
SwrContext *AudioContext::InitSwrContext(int sampleRate, AVSampleFormat sampleFormat,
                                         AVChannelLayout chLayout, AVCodecContext *enCodecCtx)
{
    SwrContext *swrCtx = swr_alloc();
    if (!swrCtx)
    {
        ret = AVERROR(ENOMEM);
        av_log_error("could not allocate resampler context");
        return swrCtx;
    }
    // set swrcontext essential parameters
    av_opt_set_chlayout(swrCtx, "in_chlayout", &chLayout, 0);
    av_opt_set_int(swrCtx, "in_sample_rate", sampleRate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", sampleFormat, 0);
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
SwrContext* AudioContext::InitSwrContext(AVCodecContext* deCodecCtx, int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout)
{

    SwrContext* swrCtx = swr_alloc();
    if (!swrCtx)
    {
        ret = AVERROR(ENOMEM);
        av_log_error("could not allocate resampler context");
        return swrCtx;
    }
    // set swrcontext essential parameters
    av_opt_set_chlayout(swrCtx, "in_chlayout", &deCodecCtx->ch_layout, 0);
    av_opt_set_int(swrCtx, "in_sample_rate", deCodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", deCodecCtx->sample_fmt, 0);
    av_opt_set_chlayout(swrCtx, "out_chlayout", &chLayout, 0);
    av_opt_set_int(swrCtx, "out_sample_rate", sampleRate, 0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", sampleFormat, 0);

    ret = swr_init(swrCtx);
    if (ret < 0)
    {
        av_log_error("failed to initialize the resampling context\n");
    }
    return swrCtx;
}
bool AudioContext::VariableCheck()
{
    if (ret < 0)
    {
        av_log_error("construct error\n");
        return false;
    }
    if (deAudioFrame == nullptr)
    {
        deAudioFrame = AllocAVFrame();
        av_log_info("devideo frame is nullptr,allocting avframe succeed");
    }
    if (enAudioCodecCtx != nullptr) {
        if (srcSampleFormat != enAudioCodecCtx->sample_fmt ||
            srcSampleRate != enAudioCodecCtx->sample_rate ||
            av_channel_layout_compare(&srcChlayout, &enAudioCodecCtx->ch_layout))
        {
            if (swrCtx != nullptr)
                swr_free(&swrCtx);
            swrCtx = InitSwrContext(srcSampleRate, srcSampleFormat, srcChlayout, enAudioCodecCtx);
            av_log_info("swrcontext is nullptr,alloced swrcontext\n");
        }
        if (swrCtx == nullptr)
        {
            av_frame_free(&enAudioFrame);
            enAudioFrame = deAudioFrame;
        }
        if (enAudioPacket == nullptr)
        {
            enAudioPacket = AllocAVPacket();
        }
    }
    else {
        av_log_info("warning:enaudio codeccontext is nullptr\n");
    }
    return true;
}
AudioContext::AudioContext(AVCodecID dstCodecId) : enAudioCodecCtx(OpenAudioEncodecContext(dstCodecId)),
                                                   outAudioFmtCtx(nullptr), deAudioFrame(nullptr), enAudioFrame(nullptr),
                                                   enAudioPacket(nullptr), swrCtx(nullptr),
                                                   maxDstNbSamples(0), pts(0), 
                                                   srcSampleRate(0), srcSampleFormat(), srcChlayout()
{
}
AudioContext::AudioContext(int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout) : AudioContext(sampleRate, sampleFormat, chLayout, DEFAULTAUDIOCODECID)
{
}
AudioContext::AudioContext(int sampleRate, AVSampleFormat sampleFormat,
                           AVChannelLayout chLayout, AVCodecID dstCodecId)
    : enAudioCodecCtx(OpenAudioEncodecContext(dstCodecId)),
      swrCtx(InitSwrContext(sampleRate, sampleFormat, chLayout, enAudioCodecCtx)),
      outAudioFmtCtx(nullptr), deAudioFrame(AllocAVFrame()), enAudioPacket(nullptr),
      enAudioFrame(nullptr), maxDstNbSamples(0), pts(0),
      srcSampleRate(sampleRate),srcSampleFormat(sampleFormat),srcChlayout(chLayout)
{
    deAudioFrame->sample_rate = sampleRate;
}
void AudioContext::WriteAudioPreparition(const char *dstFilePath)
{
    if (deAudioFrame == nullptr)
        deAudioFrame = AllocAVFrame();

    outAudioFmtCtx = AllocOutFormatContext(dstFilePath);
    if (outAudioFmtCtx == nullptr)
    {
        ret = -1;
        av_log_error("alloc outformat context failed\n");
        return;
    }
    AddNewStreamToFormat(outAudioFmtCtx, enAudioCodecCtx);
    outAudioStream = outAudioFmtCtx->streams[outAudioFmtCtx->nb_streams - 1];
    // open target file stream
    ret = avio_open(&outAudioFmtCtx->pb, dstFilePath, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        av_log_error("avio open failed\n");
        return;
    }
    // write file header information
    ret = avformat_write_header(outAudioFmtCtx, nullptr);
    if (ret < 0)
    {
        av_log_error("write header information failed\n");
        return;
    }
}
void AudioContext::WriteAudioToFile(AVFrame* deAudioFrame)
{
    for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i) {
        this->deAudioFrame->data[i] = deAudioFrame->data[i];
        this->deAudioFrame->linesize[i] = deAudioFrame->linesize[i];
    }
    this->deAudioFrame->nb_samples = deAudioFrame->nb_samples;
    ResampleDeAudioFrame();
    EncodeAudioFrame();
}
void AudioContext::WriteAudioToFile(void *data, int length)
{
    deAudioFrame->data[0] = (uint8_t *)data;
    deAudioFrame->data[1] = (uint8_t *)((float *)data + length);
    deAudioFrame->nb_samples = length;
    ResampleDeAudioFrame();
    EncodeAudioFrame();
}
void AudioContext::WriteAudioToFile()
{
    ResampleDeAudioFrame();
    EncodeAudioFrame();
}
void AudioContext::WriteAudioTrailer()
{
    ret = av_write_trailer(outAudioFmtCtx);
    if (ret < 0)
    {
        av_log_error("write trailer information failed\n");
        return;
    }
    ret = avio_close(outAudioFmtCtx->pb);
    if (ret < 0)
    {
        av_log_error("close outfmt context failed\n");
        return;
    }
    avformat_free_context(outAudioFmtCtx);
    outAudioFmtCtx = nullptr;
    outAudioStream = nullptr;

    av_frame_free(&deAudioFrame);
    deAudioFrame = nullptr;
    // av_frame_free(&enAudioFrame);
    // enAudioFrame = nullptr;
    // av_packet_free(&enAudioPacket);
    // enAudioPacket = nullptr;
    pts = 0;
}
void AudioContext::FlushEnAudioCodecBuffer()
{
    av_frame_free(&enAudioFrame);
    enAudioFrame = nullptr;
    EncodeAudioFrame();
}
AudioContext::~AudioContext()
{
    // todo ??
    swr_free(&swrCtx);
    avcodec_free_context(&enAudioCodecCtx);
}