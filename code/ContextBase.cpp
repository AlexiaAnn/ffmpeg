#include "ContextBase.h"
void UnityLogCallbackFunc(void *ptr, int level, const char *fmt, va_list vl)
{
    try
    {
        char acLogStr[10240];
        vsprintf(acLogStr, fmt, vl);
        Debug::Log(acLogStr);
    }
    catch (const std::exception &e)
    {
        Debug::Log(e.what());
    }
}
void LogCallbackTotxt(void *ptr, int level, const char *fmt, va_list vl)
{
    FILE *fp = fopen("f:/ffmpeglog.txt", "a+");
    if (fp)
    {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}
AVFormatContext *ContextBase::GetFormatContextByFileName(const char *fileName)
{
    AVFormatContext *videoContext = nullptr;
    ret = avformat_open_input(&videoContext, fileName, nullptr, nullptr);
    if (ret < 0)
    {
        av_log_info("could not open video file\n");
    }
    return videoContext;
}
AVFormatContext *ContextBase::GetOutFormatContextByFileName(const char *dstFileName, const AVCodecContext *enAudioCodecCtx, const AVCodecContext *enVideoCodecCtx)
{
    AVFormatContext *context = nullptr;
    ret = avformat_alloc_output_context2(&context, nullptr, nullptr, dstFileName);
    if (ret < 0)
    {
        av_log_error("AVOutputFormatContext alloc failed\n");
        return context;
    }
    if (enAudioCodecCtx != nullptr)
    {
        AVStream *audioStream = avformat_new_stream(context, nullptr);
        if (audioStream == nullptr)
        {
            ret = -1;
            av_log_error("create a stream to formatcontext failed\n");
            return context;
        }
        ret = avcodec_parameters_from_context(audioStream->codecpar, enAudioCodecCtx);
        if (ret < 0)
        {
            av_log_error("copy codec parameters from context failed\n");
            return context;
        }
        audioStream->id = 0;
        audioStream->codecpar->codec_tag = 0;
        audioStream->time_base = enAudioCodecCtx->time_base;
    }
    if (enVideoCodecCtx != nullptr)
    {
        AVStream *videoStream = avformat_new_stream(context, nullptr);
        if (videoStream == nullptr)
        {
            ret = -1;
            av_log_error("create a stream to formatcontext failed\n");
            return context;
        }
        // av_log_info("outputformat videostream id %d", videoStream->id);
        ret = avcodec_parameters_from_context(videoStream->codecpar, enVideoCodecCtx);
        if (ret < 0)
        {
            av_log_error("copy codec parameters from context failed\n");
            return context;
        }
        ret = av_dict_set(&videoStream->metadata, "rotate", "180", 0);

        if (ret < 0)
        {
            av_log_error("set rotate  failed\n");
            return context;
        }
        videoStream->id = enAudioCodecCtx == nullptr ? 0 : 1;
        videoStream->codecpar->codec_tag = 0;
        videoStream->time_base = enVideoCodecCtx->time_base;
    }
    return context;
}
void ContextBase::FindStreamInformation(AVFormatContext *formatContext)
{
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0)
    {
        av_log_info("could not find stream information\n");
    }
}
AVCodecContext *ContextBase::OpenVideoEncodecContext(AVCodecID codecId, int fps, int width, int height)
{
    AVCodecContext *context = AllocEnCodecContext(codecId);
    if (codecId == AV_CODEC_ID_H264)
    {
        context->bit_rate = (width * height * 3 / 2);
        context->pix_fmt = AV_PIX_FMT_YUV420P;
    }
    context->width = width;
    context->height = height;
    context->rc_buffer_size = (int)context->bit_rate;
    context->framerate.num = fps;
    context->framerate.den = 1;
    context->time_base.num = 1;
    context->time_base.den = fps;
    context->gop_size = fps;
    context->max_b_frames = 0;
    context->has_b_frames = 0;
    context->codec_id = codecId;
    context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    const AVCodec *codec = avcodec_find_encoder(codecId);
    av_log_info("is opening video codec context\n");
    ret = avcodec_open2(context, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec context\n");
    }
    av_log_info("open video codec context end\n");
    return context;
}
SwsContext *ContextBase::AllocSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight, AVCodecContext *enCodecCtx)
{
    SwsContext *context = nullptr;
    // context = sws_getCachedContext(context, width, height, dePixFormat,
    //                                enCodecCtx->width, enCodecCtx->height, enCodecCtx->pix_fmt,
    //                                SWS_BICUBIC, nullptr, nullptr, nullptr);
    context = sws_getContext(deWidth, deHeight, dePixFormat,
                             enCodecCtx->width, enCodecCtx->height, enCodecCtx->pix_fmt,
                             SWS_BICUBIC, nullptr, nullptr, nullptr);
    return context;
}
AVFrame *ContextBase::InitVideoAVFrame(AVCodecContext *codecContxt)
{
    AVFrame *frame = AllocAVFrame();
    frame->format = codecContxt->pix_fmt;
    frame->width = codecContxt->width;
    frame->height = codecContxt->height;
    ret = av_frame_get_buffer(frame, 0); // error prone
    if (ret < 0)
    {
        av_log_error("could not allocate the frame data buffer\n");
    }
    return frame;
}
AVCodecContext *ContextBase::OpenAudioEncodecContext(AVCodecID codecId)
{
    AVCodecContext *codecContext = AllocEnCodecContext(codecId);
    if (codecId == AV_CODEC_ID_MP3)
    {
        codecContext->bit_rate = 192000;
        codecContext->sample_rate = 48000;
        AVChannelLayout layout = {AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO}};
        av_channel_layout_copy(&codecContext->ch_layout, &layout);
    }
    else
    {
        av_log_error("audio type now is not supported\n");
        ret = -1;
    }
    const AVCodec *codec = avcodec_find_encoder(codecId);
    codecContext->rc_buffer_size = (int)codecContext->bit_rate;
    codecContext->sample_fmt = codec->sample_fmts[0];
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec\n");
    }
    return codecContext;
}
SwrContext *ContextBase::AllocSwrContext(AVChannelLayout layout, int sampleRate, AVSampleFormat sampleFmt, AVCodecContext *enCodecCtx)
{
    SwrContext *swrCtx = swr_alloc();
    if (!swrCtx)
    {
        ret = AVERROR(ENOMEM);
        av_log_error("could not allocate resampler context");
        return swrCtx;
    }
    // set swrcontext essential parameters
    av_opt_set_chlayout(swrCtx, "in_chlayout", &layout, 0);
    av_opt_set_int(swrCtx, "in_sample_rate", sampleRate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", sampleFmt, 0);
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
SwrContext *ContextBase::AllocSwrContext(AVCodecContext *deCodecCtx, AVCodecContext *enCodecCtx)
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
void ContextBase::InitAudioFrame(AVFrame *&frame, AVCodecContext *codecCtx, int dstNbSamples)
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
AVFrame *ContextBase::InitAudioFrame(AVCodecContext *codecCtx, int dstNbSamples)
{
    AVFrame *frame = AllocAVFrame();
    frame->sample_rate = codecCtx->sample_rate;
    frame->format = codecCtx->sample_fmt;
    av_channel_layout_copy(&frame->ch_layout, &codecCtx->ch_layout);
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
AVFrame *ContextBase::AllocAVFrame()
{
    AVFrame *frame = av_frame_alloc();
    if (!frame)
    {
        ret = -1;
        av_log_error("could not allocate frame");
    }
    return frame;
}

AVPacket *ContextBase::AllocAVPacket()
{
    AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
        ret = -1;
        av_log_error("could not allocate packet");
    }
    return packet;
}

AVCodecContext *ContextBase::OpenDecodecContextByStreamPar(AVStream *stream)
{
    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
    {
        ret = AVERROR(EINVAL);
        av_log_error("failed to find target codec\n");
        return nullptr;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        ret = AVERROR(ENOMEM);
        av_log_error("failed to allocate target codec context\n");
        return codecContext;
    }
    ret = avcodec_parameters_to_context(codecContext, stream->codecpar);
    if (ret < 0)
    {
        av_log_error("failed to copy codecpar to codec context");
        return codecContext;
    }
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("failed to init codec context\n");
    }
    return codecContext;
}

AVCodecContext *ContextBase::AllocDecodecContext(AVCodecID codecId)
{
    const AVCodec *codec = avcodec_find_decoder(codecId);
    if (!codec)
    {
        ret = -1;
        av_log_error("codec not found\n");
        return nullptr;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        ret = -1;
        av_log_error("could not allocate codec context\n");
    }
    return codecContext;
}

AVCodecContext *ContextBase::AllocEnCodecContext(AVCodecID codecId)
{
    const AVCodec *codec = avcodec_find_encoder(codecId);
    if (!codec)
    {
        ret = -1;
        av_log_error("codec not found\n");
        return nullptr;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        ret = -1;
        av_log_error("could not allocate codec context\n");
    }
    return codecContext;
}

void ContextBase::DecodePacket(AVFormatContext *formatContext)
{
    int audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    int videoStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    while (av_read_frame(formatContext, dePacket) >= 0)
    {
        if (dePacket->stream_index == audioStreamIndex)
        {
            DealAudioPacket(dePacket);
        }
        else if (dePacket->stream_index == videoStreamIndex)
        {
            DealVideoPacket(dePacket);
        }
    }
    av_log_info("decode all packet,over\n");
}

void ContextBase::DealAudioPacket(AVPacket *packet)
{
}

void ContextBase::DealVideoPacket(AVPacket *packet)
{
}
std::pair<int, int> ContextBase::FindAVStreamIndex() const
{
    int audioStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    int videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    return {audioStreamIndex, videoStreamIndex};
}
ContextBase::ContextBase() : fmtCtx(nullptr), ret(0), dePacket(nullptr)
{
}
ContextBase::ContextBase(const char *srcFilePath) : ret(0), dePacket(nullptr)
{
    // av_log_set_callback(LogCallbackTotxt);
    fmtCtx = GetFormatContextByFileName(srcFilePath);
    FindStreamInformation(fmtCtx);
}
void ContextBase::ReSetAVFormatContext(const char *srcFilePath)
{
    avformat_free_context(fmtCtx);
    fmtCtx = GetFormatContextByFileName(srcFilePath);
    FindStreamInformation(fmtCtx);
}

void ContextBase::DumpFileInformation(const char *fileName) const
{
    av_dump_format(fmtCtx, 0, fileName, 0);
}

const AVFormatContext *ContextBase::GetFormatContext() const
{
    return fmtCtx;
}

ContextBase::~ContextBase()
{
    av_packet_free(&dePacket);
    avformat_free_context(fmtCtx);
}
void (*Debug::LogFunPtr)(char *message, int iSize);
void (*Debug::LogErrorFunPtr)(char *message, int iSize);
void Debug::Log(const char *fmt, ...)
{
    if (Debug::LogFunPtr == NULL)
        return;
    char acLogStr[10240]; // = { 0 }; error prone
    va_list ap;
    va_start(ap, fmt);
    vsprintf(acLogStr, fmt, ap);
    va_end(ap);
    Debug::LogFunPtr(acLogStr, strlen(acLogStr));
}

void Debug::LogError(const char *msg, ...)
{
    if (Debug::LogErrorFunPtr == NULL)
        return;
    char acLogStr[10240]; // = { 0 }; error prone
    va_list ap;
    va_start(ap, msg);
    vsprintf(acLogStr, msg, ap);
    va_end(ap);
    Debug::LogErrorFunPtr(acLogStr, strlen(acLogStr));
}
