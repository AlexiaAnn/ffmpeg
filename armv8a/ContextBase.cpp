#include "ContextBase.h"
void UnityLogCallbackFunc(void* ptr, int level, const char* fmt, va_list vl)
{
    char printf_buf[1024];
    snprintf(printf_buf, 1024, fmt, vl);
    Debug::Log(printf_buf);
}
void LogCallbackTotxt(void* ptr, int level, const char* fmt, va_list vl)
{
    FILE* fp = fopen("e:/ffmpegframetest.txt", "a+");
    if (fp)
    {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}
AVFormatContext* ContextBase::GetFormatContextByFileName(const char* fileName)
{
    AVFormatContext* videoContext = nullptr;
    ret = avformat_open_input(&videoContext, fileName, nullptr, nullptr);
    if (ret < 0)
    {
        av_log_info("could not open video file\n");
    }
    return videoContext;
}
AVFormatContext* ContextBase::GetOutFormatContextByFileName(const char* dstFileName, AVCodecContext* enAudioCodecCtx, AVCodecContext* enVideoCodecCtx)
{
    AVFormatContext* context = nullptr;
    ret = avformat_alloc_output_context2(&context, nullptr, nullptr, dstFileName);
    if (ret < 0)
    {
        av_log_error("AVOutputFormatContext alloc failed\n");
        return context;
    }
    if (enAudioCodecCtx != nullptr)
    {
        AVStream* audioStream = avformat_new_stream(context, nullptr);
        if (audioStream == nullptr)
        {
            ret = -1;
            av_log_error("create a stream to formatcontext failed\n");
            return context;
        }
        audioStream->id = 0;
        // audioStream->codecpar->codec_tag = 0;//error prone
        ret = avcodec_parameters_from_context(audioStream->codecpar, enAudioCodecCtx);
        if (ret < 0)
        {
            av_log_error("copy codec parameters from context failed\n");
            return context;
        }
    }
    if (enVideoCodecCtx != nullptr)
    {
        AVStream* videoStream = avformat_new_stream(context, nullptr);
        if (videoStream == nullptr)
        {
            ret = -1;
            av_log_error("create a stream to formatcontext failed\n");
            return context;
        }
        videoStream->id = enAudioCodecCtx == nullptr ? 0 : 1;
        av_log_info("outputformat videostream id %d", videoStream->id);
        avcodec_parameters_from_context(videoStream->codecpar, enVideoCodecCtx);
        if (ret < 0)
        {
            av_log_error("copy codec parameters from context failed\n");
            return context;
        }
    }
    return context;
}
void ContextBase::FindStreamInformation(AVFormatContext* formatContext)
{
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0)
    {
        av_log_info("could not find stream information\n");
    }
}

AVFrame* ContextBase::AllocAVFrame()
{
    AVFrame* frame = av_frame_alloc();
    if (!frame)
    {
        ret = -1;
        av_log_error("could not allocate frame");
    }
    return frame;
}

AVPacket* ContextBase::AllocAVPacket()
{
    AVPacket* packet = av_packet_alloc();
    if (!packet)
    {
        ret = -1;
        av_log_error("could not allocate packet");
    }
    return packet;
}

AVCodecContext* ContextBase::OpenDecodecContextByStreamPar(AVStream* stream)
{
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
    {
        ret = AVERROR(EINVAL);
        av_log_error("failed to find target codec\n");
        return nullptr;
    }
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
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

AVCodecContext* ContextBase::AllocDecodecContext(AVCodecID codecId)
{
    const AVCodec* codec = avcodec_find_decoder(codecId);
    if (!codec)
    {
        ret = -1;
        av_log_error("codec not found\n");
        return nullptr;
    }
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        ret = -1;
        av_log_error("could not allocate codec context\n");
    }
    return codecContext;
}

AVCodecContext* ContextBase::AllocEnCodecContext(AVCodecID codecId)
{
    const AVCodec* codec = avcodec_find_encoder(codecId);
    if (!codec)
    {
        ret = -1;
        av_log_error("codec not found\n");
        return nullptr;
    }
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        ret = -1;
        av_log_error("could not allocate codec context\n");
    }
    return codecContext;
}

void ContextBase::DecodePacket(AVFormatContext* formatContext)
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
    av_log_info("decode all packet,over");
}

void ContextBase::DealAudioPacket(AVPacket* packet)
{
}

void ContextBase::DealVideoPacket(AVPacket* packet)
{
}

ContextBase::ContextBase() : fmtCtx(nullptr), ret(0), dePacket(nullptr)
{
}
ContextBase::ContextBase(const char* srcFilePath) : ret(0), dePacket(nullptr)
{
    fmtCtx = GetFormatContextByFileName(srcFilePath);
}
void ContextBase::ReSetAVFormatContext(const char* srcFilePath)
{
    avformat_free_context(fmtCtx);
    fmtCtx = GetFormatContextByFileName(srcFilePath);
    FindStreamInformation(fmtCtx);
}

void ContextBase::DumpFileInformation(const char* fileName) const
{
    av_dump_format(fmtCtx, 0, fileName, 0);
}

ContextBase::~ContextBase()
{
    av_packet_free(&dePacket);
    avformat_free_context(fmtCtx);
}
void (*Debug::LogFunPtr)(char* message, int iSize);
void Debug::Log(const char* fmt, ...)
{
    if (Debug::LogFunPtr == NULL)
        return;
    char acLogStr[512]; // = { 0 };
    va_list ap;
    va_start(ap, fmt);
    vsprintf(acLogStr, fmt, ap);
    va_end(ap);
    Debug::LogFunPtr(acLogStr, strlen(acLogStr));
}