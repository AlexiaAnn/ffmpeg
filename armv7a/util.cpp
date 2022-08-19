
#include "util.h"

AVFormatContext *GetFormatContextByFileName(const char *inVideoFileName, std::shared_ptr<spdlog::logger> logger)
{
    AVFormatContext *videoContext = nullptr;
    int ret = avformat_open_input(&videoContext, inVideoFileName, nullptr, nullptr);
    if (ret < 0)
    {
        av_log_info("could not open video file\n");
        return nullptr;
    }
    return videoContext;
}
bool Find_stream_info(AVFormatContext *formatContext, std::shared_ptr<spdlog::logger> logger)
{
    if (avformat_find_stream_info(formatContext, nullptr) < 0)
    {
        av_log_info("could not find stream information\n");
        avformat_close_input(&formatContext);
        return false;
    }
    return true;
}
void GetVideoInformation(const char *filePath)
{
    AVFormatContext *context = GetFormatContextByFileName(filePath, nullptr);
    av_dump_format(context, 0, filePath, 0);
    avformat_free_context(context);
}
AVFrame *CreateAvFrame()
{
    AVFrame *frame = av_frame_alloc();
    if (!frame)
    {
        av_log_error("could not allocate frame");
    }
    return frame;
}
AVPacket *CreateAvPacket()
{
    AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
        av_log_error("could not allocate packet");
    }
    return packet;
}
int Open_decodec_context(AVStream *stream, AVCodecContext **codecContext, AVFormatContext *fmtContext, AVMediaType type, std::shared_ptr<spdlog::logger> logger)
{
    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
    {
        av_log_error("failed to find target codec\n");
        return AVERROR(EINVAL);
    }
    *codecContext = avcodec_alloc_context3(codec);
    if (!*codecContext)
    {
        av_log_error("failed to allocate target codec context\n");
        return AVERROR(ENOMEM);
    }
    int ret = avcodec_parameters_to_context(*codecContext, stream->codecpar);
    if (ret < 0)
    {
        av_log_error("failed to copy codecpar to codec context");
        return ret;
    }
    if ((ret = avcodec_open2(*codecContext, codec, nullptr)) < 0)
    {
        av_log_error("failed to init codec context\n");
        return ret;
    }
    return 0;
}

int Open_encodec_context(AVStream *stream, AVCodecID targetCodecID, AVCodecContext **codecContext, std::shared_ptr<spdlog::logger> logger)
{
    const AVCodec *codec = avcodec_find_encoder(targetCodecID);
    if (!codec)
    {
        av_log_error("codec not found\n");
        return -1;
    }
    *codecContext = avcodec_alloc_context3(codec);
    if (!*codecContext)
    {
        av_log_error("could not allocate codec context\n");
        return -1;
    }
    (*codecContext)->bit_rate = 192000;
    (*codecContext)->sample_fmt = codec->sample_fmts[0];
    (*codecContext)->sample_rate = 48000;
    av_channel_layout_copy(&((*codecContext)->ch_layout), &(stream->codecpar->ch_layout));
    /* open it */
    int ret = avcodec_open2(*codecContext, codec, NULL);
    if (ret < 0)
    {
        av_log_error("could not open codec\n");
        return ret;
    }
    return 0;
}

AVCodecContext *AllocEncodecContext(AVCodecID codecID)
{
    const AVCodec *codec = avcodec_find_encoder(codecID);
    if (!codec)
    {
        av_log_error("codec not found\n");
        return nullptr;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        av_log_error("could not allocate codec context\n");
    }
    return codecContext;
}

int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry
    {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        {AV_SAMPLE_FMT_U8, "u8", "u8"},
        {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
        {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
        {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
        {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
        {AV_SAMPLE_FMT_FLTP, "f32be", "f32le"},
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++)
    {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt)
        {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}
