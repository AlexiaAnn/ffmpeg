#include "util.h"
AVPacket *AllocAVPacket()
{
    AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
        av_log_error("could not allocate packet");
        return nullptr;
    }
    return packet;
}
AVFrame *AllocAVFrame()
{
    AVFrame *frame = av_frame_alloc();
    if (!frame)
    {
        av_log_error("could not allocate frame");
        return nullptr;
    }
    return frame;
}
AVFormatContext *GetFormatContextByFileName(const char *fileName)
{
    AVFormatContext *videoContext = nullptr;
    int ret = avformat_open_input(&videoContext, fileName, nullptr, nullptr);
    if (ret < 0)
    {
        av_log_info("could not open video file\n");
        return nullptr;
    }
    return videoContext;
}

AVFormatContext *AllocOutFormatContext(const char *dstFileName)
{
    AVFormatContext *context = nullptr;
    int ret = avformat_alloc_output_context2(&context, nullptr, nullptr, dstFileName);
    if (ret < 0)
    {
        av_log_error("AVOutputFormatContext alloc failed\n");
        return nullptr;
    }
    return context;
}

int AddNewStreamToFormat(AVFormatContext *outFormatContext, AVCodecContext *codecContext)
{
    AVStream *stream = avformat_new_stream(outFormatContext, nullptr);
    if (stream == nullptr)
    {
        av_log_error("create a stream to formatcontext failed\n");
        return -1;
    }
    int ret = avcodec_parameters_from_context(stream->codecpar, codecContext);
    if (ret < 0)
    {
        av_log_error("copy codec parameters from context failed\n");
        return ret;
    }
    stream->id = outFormatContext->nb_streams - 1;
    stream->codecpar->codec_tag = 0;
    stream->time_base = codecContext->time_base; // error prone
    return ret;
}
AVCodecContext *AllocEncodecContext(AVCodecID codecId)
{
    const AVCodec *codec = avcodec_find_encoder(codecId);
    if (!codec)
    {
        av_log_error("codec not found\n");
        return nullptr;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        av_log_error("could not allocate codec context\n");
        return nullptr;
    }
    return codecContext;
}
AVCodecContext *OpenDecodecContextByStream(AVStream *stream)
{
    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
    {
        av_log_error("failed to find target codec\n");
        return nullptr;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        av_log_error("failed to allocate target codec context\n");
        return codecContext;
    }
    int ret = avcodec_parameters_to_context(codecContext, stream->codecpar);
    if (ret < 0)
    {
        av_log_error("failed to copy codecpar to codec context");
        avcodec_free_context(&codecContext);
        return nullptr;
    }
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("failed to init codec context\n");
        avcodec_free_context(&codecContext);
        return nullptr;
    }
    return codecContext;
}

const char* GetSampleFormatString(AVSampleFormat format)
{
    switch (format)
    {
    case AV_SAMPLE_FMT_NONE:
        return "AV_SAMPLE_FMT_NONE";
    case AV_SAMPLE_FMT_U8:
        return "AV_SAMPLE_FMT_U8";
    case AV_SAMPLE_FMT_S16:
        return "AV_SAMPLE_FMT_S16";
    case AV_SAMPLE_FMT_S32:
        return "AV_SAMPLE_FMT_S32";
    case AV_SAMPLE_FMT_FLT:
        return "AV_SAMPLE_FMT_FLT";
    case AV_SAMPLE_FMT_DBL:
        return "AV_SAMPLE_FMT_DBL";
    case AV_SAMPLE_FMT_U8P:
        return "AV_SAMPLE_FMT_U8P";
    case AV_SAMPLE_FMT_S16P:
        return "AV_SAMPLE_FMT_S16P";
    case AV_SAMPLE_FMT_S32P:
        return "AV_SAMPLE_FMT_S32P";
    case AV_SAMPLE_FMT_FLTP:
        return "AV_SAMPLE_FMT_FLTP";
    case AV_SAMPLE_FMT_DBLP:
        return "AV_SAMPLE_FMT_DBLP";
    case AV_SAMPLE_FMT_S64:
        return "AV_SAMPLE_FMT_S64";
    case AV_SAMPLE_FMT_S64P:
        return "AV_SAMPLE_FMT_S64P";
    case AV_SAMPLE_FMT_NB:
        return "AV_SAMPLE_FMT_NB";
    }
    return "AV_SAMPLE_FMT_NONE";
}

AVFilterInOut* AllocAVFilterInOut() {
    AVFilterInOut* inout = avfilter_inout_alloc();
    return inout;
}
AVFilterGraph* AllocAVFilterGraph() {
    AVFilterGraph* graph = avfilter_graph_alloc();
    return graph;
}
AVFrame* CreateVideoFrame(AVPixelFormat format, int width, int height) {
    AVFrame* frame = AllocAVFrame();
    frame->format = format;
    frame->width = width;
    frame->height = height;
    int ret = av_frame_get_buffer(frame, 0); // error prone
    if (ret < 0)
    {
        av_log_error("could not allocate the frame data buffer\n");
    }
    return frame;
}

void FlipImage(unsigned char* src,int width,int height)
{
    unsigned char* tempSrc = NULL;
    int srcW = width;
    int srcH = height;
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
}
