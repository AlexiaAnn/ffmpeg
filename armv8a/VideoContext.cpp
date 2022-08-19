#include "VideoContext.h"

AVCodecContext *VideoContext::OpenVideoEncodecContext(AVCodecID codecId, int fps, int width, int height)
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
    context->time_base.den = fps * 1000;
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

SwsContext *VideoContext::AllocSwsContext(AVCodecContext *deCodecCtx, AVCodecContext *enCodecCtx)
{
    SwsContext *context = nullptr;
    context = sws_getCachedContext(context, deCodecCtx->width, deCodecCtx->height, deCodecCtx->pix_fmt,
                                   enCodecCtx->width, enCodecCtx->height, enCodecCtx->pix_fmt,
                                   SWS_BICUBIC, nullptr, nullptr, nullptr);
    return context;
}
SwsContext *VideoContext::AllocSwsContext(AVPixelFormat dePixFormat, int width, int height, AVCodecContext *enCodecCtx)
{
    SwsContext *context = nullptr;
    // context = sws_getCachedContext(context, width, height, dePixFormat,
    //                                enCodecCtx->width, enCodecCtx->height, enCodecCtx->pix_fmt,
    //                                SWS_BICUBIC, nullptr, nullptr, nullptr);
    context = sws_getContext(width, height, dePixFormat,
                             enCodecCtx->width, enCodecCtx->height, enCodecCtx->pix_fmt,
                             SWS_BICUBIC, nullptr, nullptr, nullptr);
    return context;
}
AVFrame *VideoContext::CreateVideoAVFrame(AVCodecContext *codecContext)
{
    AVFrame *frame = AllocAVFrame();
    frame->format = enVideoCodecCtx->pix_fmt;
    frame->width = enVideoCodecCtx->width;
    frame->height = enVideoCodecCtx->height;
    ret = av_frame_get_buffer(frame, 0); // error prone
    if (ret < 0)
    {
        av_log_error("could not allocate the frame data buffer\n");
    }
    return frame;
}
void VideoContext::RescaleDeVideoFrame()
{
    ret = sws_scale(swsCtx, deVideoFrame->data, deVideoFrame->linesize, 0,
                    enVideoCodecCtx->height, enVideoFrame->data, enVideoFrame->linesize);
    av_log_info("sws scale result:%d", ret);
    // av_log_info("deFrame address:%d,deFrame size:%d,enFrame address:%d,enFrame size:%d", deVideoFrame->data[0], deVideoFrame->pkt_size, enVideoFrame->data[0], enVideoFrame->pkt_size);
    if (ret <= 0)
    {
        av_log_error("sws scale the video frame failed\n");
        return;
    }
}
void VideoContext::RescaleDevideoFrame(uint8_t **data, int *linesize)
{
    ret = sws_scale(swsCtx, data, linesize, 0,
                    enVideoCodecCtx->height, enVideoFrame->data, enVideoFrame->linesize);
    av_log_info("sws scale result:%d", ret);
    if (ret <= 0)
    {
        av_log_error("sws scale the video frame failed\n");
    }
}
void VideoContext::EncodeVideoFrame()
{
    enVideoFrame->pts = pts;
    pts += 3600; // error prone
    ret = avcodec_send_frame(enVideoCodecCtx, enVideoFrame);
    av_log_info("send frame ret:%d", ret);
    if (ret < 0)
    {
        av_log_error("send frame to enVideoCodecContext error\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enVideoCodecCtx, enVideoPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_log_error("eagain averror_eof %d\n", ret);
            return;
        }

        if (ret < 0)
        {
            av_log_error("enVideoCodecContext receive packet error\n");
            return;
        }
        av_log_info("write a packet\n");
        av_interleaved_write_frame(outFmtCtx, enVideoPacket);
        // avio_flush(outFmtCtx->pb);
        // av_log_info("flush a packet\n");
        av_packet_unref(enVideoPacket);
        if (ret < 0)
            return;
    }
}
void VideoContext::EncodeVideoFrame(AVFrame *frame)
{
    if (frame != nullptr)
    {
        frame->pts = pts;
        pts += 3600; // error prone
    }
    ret = avcodec_send_frame(enVideoCodecCtx, frame);
    av_log_info("send frame ret:%d", ret);
    if (ret < 0)
    {
        av_log_error("send frame to enVideoCodecContext error\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enVideoCodecCtx, enVideoPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_log_error("eagain averror_eof %d\n", ret);
            return;
        }

        if (ret < 0)
        {
            av_log_error("enVideoCodecContext receive packet error\n");
            return;
        }
        av_log_info("write a packet\n");
        av_interleaved_write_frame(outFmtCtx, enVideoPacket);
        // avio_flush(outFmtCtx->pb);
        // av_log_info("flush a packet\n");
        av_packet_unref(enVideoPacket);
        if (ret < 0)
            return;
    }
}
void VideoContext::DealVideoPacket(AVPacket *packet)
{
    ret = avcodec_send_packet(deVideoCodecCtx, packet);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(deVideoCodecCtx, deVideoFrame);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return;

            av_log_error("Error during decoding");
            return;
        }
        RescaleDeVideoFrame();
        EncodeVideoFrame();
        av_frame_unref(deVideoFrame);
        if (ret < 0)
            return;
    }
}
void VideoContext::SetOutFormatContextByFileName(const char *dstFilepath)
{
    outFmtCtx = GetOutFormatContextByFileName(dstFilepath, nullptr, enVideoCodecCtx);
}
void VideoContext::ConsumeFrameFromtQueueToRGBFile(const char *dstFilePath)
{
    FILE *dstFilePtr = fopen(dstFilePath, "wb");
    deVideoFrame = AllocAVFrame();
    if (dstFilePtr == nullptr)
    {
        av_log_error("%s open failed\n", dstFilePath);
    }
    while (isEnd == false)
    {
        frameMutex.lock();
        if (frameQueue.empty())
        {
            frameMutex.unlock();
            continue;
        }
        av_log_info("consuming a frame from queue\n");
        std::pair<void *, int> frameData = frameQueue.front();
        frameQueue.pop();
        frameMutex.unlock();
        deVideoFrame->data[0] = (uint8_t *)frameData.first;
        deVideoFrame->linesize[0] = enVideoCodecCtx->width * 4;
        fwrite(frameData.first, 1, frameData.second, dstFilePtr);
    }
    av_log_info("add frame end,consuming the rest frame\n");
    while (!frameQueue.empty())
    {
        frameMutex.lock();
        std::pair<void *, int> frameData = frameQueue.front();
        frameQueue.pop();
        frameMutex.unlock();
        fwrite(frameData.first, 1, frameData.second, dstFilePtr);
    }
    fclose(dstFilePtr);
}
void VideoContext::WriteFramePrepare(const char *dstFilePath)
{
    // prepare
    deVideoFrame = AllocAVFrame();
    // start decode and write dst file
    outFmtCtx = GetOutFormatContextByFileName(dstFilePath, nullptr, enVideoCodecCtx);
    if (outFmtCtx == nullptr)
    {
        av_log_error("get outformat context failed\n");
        return;
    }
    // open target file stream
    ret = avio_open(&outFmtCtx->pb, dstFilePath, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        av_log_error("avio open failed\n");
        return;
    }
    // write file header information
    ret = avformat_write_header(outFmtCtx, nullptr);
    if (ret < 0)
    {
        av_log_error("write header information failed\n");
        return;
    }
}
void VideoContext::ConsumeFrameFromQueue()
{
    while (isEnd == false)
    {
        frameMutex.lock();
        if (frameQueue.empty())
        {
            frameMutex.unlock();
            continue;
        }
        av_log_info("consuming a frame from queue\n");
        std::pair<void *, int> frameData = frameQueue.front();
        frameQueue.pop();
        frameMutex.unlock();
        deVideoFrame->data[0] = (uint8_t *)frameData.first;
        deVideoFrame->linesize[0] = enVideoCodecCtx->width * 4;
        RescaleDeVideoFrame();
        EncodeVideoFrame();
        av_frame_unref(deVideoFrame);
    }
    av_log_info("add frame end,consuming the rest frame\n");
    while (!frameQueue.empty())
    {
        frameMutex.lock();
        std::pair<void *, int> frameData = frameQueue.front();
        frameQueue.pop();
        frameMutex.unlock();
        deVideoFrame->data[0] = (uint8_t *)frameData.first;
        deVideoFrame->linesize[0] = enVideoCodecCtx->width * 4;
        RescaleDeVideoFrame();
        EncodeVideoFrame();
        av_frame_unref(deVideoFrame);
    }
}
void VideoContext::WriteFrameTailer()
{
    EncodeVideoFrame(nullptr);
    ret = av_write_trailer(outFmtCtx);
    if (ret < 0)
    {
        av_log_error("write trailer information failed\n");
        return;
    }
    ret = avio_close(outFmtCtx->pb);
    if (ret < 0)
    {
        av_log_error("close outfmt context failed\n");
        return;
    }
    av_frame_free(&deVideoFrame);
    deVideoFrame = nullptr;
    avformat_free_context(outFmtCtx);
    outFmtCtx = nullptr;
}
void VideoContext::AddFrameToQueue(void *dataPtr, int length)
{
    frameMutex.lock();
    frameQueue.push(std::pair<void *, int>(dataPtr, length));
    frameMutex.unlock();
    av_log_info("added a frame to queue\n");
}
void VideoContext::WriteFrame(void *data, int length)
{
    deVideoFrame->data[0] = (uint8_t *)data;
    deVideoFrame->linesize[0] = enVideoCodecCtx->width * 4;
    RescaleDeVideoFrame();
    EncodeVideoFrame();
}
void VideoContext::WriteFrameToYUVFile(void *data, int length)
{
    uint8_t *indata[AV_NUM_DATA_POINTERS] = {0};
    indata[0] = (uint8_t *)data;
    int inlinesize[AV_NUM_DATA_POINTERS] = {0};
    inlinesize[0] = enVideoCodecCtx->width * 4;
    RescaleDevideoFrame(indata, inlinesize);
    int padding = enVideoFrame->linesize[0];

    uint32_t pitchY = enVideoFrame->linesize[0];
    uint32_t pitchU = enVideoFrame->linesize[1];
    uint32_t pitchV = enVideoFrame->linesize[2];

    uint8_t *avY = enVideoFrame->data[0];
    uint8_t *avU = enVideoFrame->data[1];
    uint8_t *avV = enVideoFrame->data[2];
    for (uint32_t i = 0; i < enVideoFrame->height; i++)
    {
        fwrite(avY, enVideoFrame->width, 1, dstFilePtr);
        avY += pitchY;
    }

    for (uint32_t i = 0; i < enVideoFrame->height / 2; i++)
    {
        fwrite(avU, enVideoFrame->width / 2, 1, dstFilePtr);
        avU += pitchU;
    }

    for (uint32_t i = 0; i < enVideoFrame->height / 2; i++)
    {
        fwrite(avV, enVideoFrame->width / 2, 1, dstFilePtr);
        avV += pitchV;
    }
}

void VideoContext::WriteFrameFromRGBFile(void *data, int length)
{
    av_log_info("write frame start");
    int size = enVideoCodecCtx->width * enVideoCodecCtx->height * 4;
    unsigned char *rgb = new unsigned char[size];
    int len = fread(rgb, 1, size, srcFilePtr);
    av_log_info("len:%d", len);
    if (len <= 0)
    {
        return;
    }
    deVideoFrame->data[0] = rgb;
    deVideoFrame->linesize[0] = enVideoCodecCtx->width * 4;
    RescaleDeVideoFrame();
    EncodeVideoFrame();
    av_log_info("write frame end");
}
int VideoContext::GetQueueSize()
{
    frameMutex.lock();
    int size = frameQueue.size();
    frameMutex.unlock();
    return size;
}
VideoContext::VideoContext(AVCodecID codecId, int fps, int width, int height) : ContextBase(), swsCtx(nullptr), deVideoCodecCtx(nullptr), pts(0),
                                                                                dstFilePtr(nullptr), outFmtCtx(nullptr), enCodecId(codecId), frameNumber(0)
{
    enVideoCodecCtx = OpenVideoEncodecContext(codecId, fps, width, height);
    enVideoFrame = CreateVideoAVFrame(enVideoCodecCtx);
    enVideoPacket = AllocAVPacket();
}

VideoContext::VideoContext(AVPixelFormat dePixFormat, AVCodecID codecId, int fps, int width, int height) : VideoContext(codecId, fps, width, height)
{
    isEnd = false;
    swsCtx = AllocSwsContext(dePixFormat, width, height, enVideoCodecCtx);
}
VideoContext::VideoContext(AVPixelFormat dePixFormat, AVCodecID codecId, int fps, int deWidth, int deHeight, int enWidth, int enHeight) : VideoContext(codecId, fps, enWidth, enHeight)
{
    isEnd = false;
    swsCtx = AllocSwsContext(dePixFormat, deWidth, deHeight, enVideoCodecCtx);
}
bool VideoContext::ExtractVideoToFile(const char *srcFilePath, const char *dstFilePath)
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
    // set decodecontext
    int videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVStream *videoStream = fmtCtx->streams[videoStreamIndex];
    AVCodecID deCodecId = videoStream->codecpar->codec_id;
    if (deVideoCodecCtx == nullptr)
    {
        deVideoCodecCtx = OpenDecodecContextByStreamPar(videoStream);
        av_log_info("deCodecContext is nullptr,now open the deCodecContext\n");
    }
    else
    {
        if (deCodecId != deVideoCodecCtx->codec_id)
        {
            // reset decodeccontxt
            avcodec_free_context(&deVideoCodecCtx);
            deVideoCodecCtx = OpenDecodecContextByStreamPar(videoStream);
            av_log_info("deCodecContext need to change,reopen the deCodecContext\n");
        }
    }
    // set swscontext
    if (swsCtx == nullptr)
    {
        swsCtx = AllocSwsContext(deVideoCodecCtx, enVideoCodecCtx);
        av_log_info("swrcontext is nullptr,alloced swrcontext\n");
    }
    else
    {
        sws_freeContext(swsCtx);
        swsCtx = AllocSwsContext(deVideoCodecCtx, enVideoCodecCtx);
        av_log_info("swrcontext need to change,reset swrcontext\n");
    }
    deVideoFrame = AllocAVFrame();
    // start decode and write dst file
    outFmtCtx = GetOutFormatContextByFileName(dstFilePath, nullptr, enVideoCodecCtx);
    // open target file stream
    ret = avio_open(&outFmtCtx->pb, dstFilePath, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        av_log_error("avio open failed\n");
        return false;
    }
    // write file header information
    ret = avformat_write_header(outFmtCtx, nullptr);
    if (ret < 0)
    {
        av_log_error("write header information failed\n");
        return false;
    }
    DecodePacket(fmtCtx);
    // write file trailer information
    ret = av_write_trailer(outFmtCtx);
    if (ret < 0)
    {
        av_log_error("write trailer information failed\n");
        return false;
    }
    av_frame_free(&deVideoFrame);
    avformat_free_context(outFmtCtx);
    return ret >= 0;
}

bool VideoContext::ExtractRGBToFile(const char *srcFilePath, const char *dstFilePath)
{
    WriteFramePrepare(dstFilePath);
    FILE *srcFilePtr = fopen(srcFilePath, "rb");
    if (srcFilePtr == nullptr)
    {
    }
    int size = enVideoCodecCtx->width * enVideoCodecCtx->height * 4;
    unsigned char *rgb = new unsigned char[size];
    while (true)
    {
        int len = fread(rgb, 1, size, srcFilePtr);
        if (len <= 0)
            break;
        deVideoFrame->data[0] = rgb;
        deVideoFrame->linesize[0] = enVideoCodecCtx->width * 4;
        RescaleDeVideoFrame();
        EncodeVideoFrame();
        av_log_info("complete a frame\n");
    }
    WriteFrameTailer();
    return false;
}

VideoContext::~VideoContext()
{
    if (swsCtx != nullptr)
        sws_freeContext(swsCtx);
    if (deVideoCodecCtx != nullptr)
        avcodec_free_context(&deVideoCodecCtx);
    av_frame_free(&enVideoFrame);
    av_packet_free(&enVideoPacket);
    avcodec_free_context(&enVideoCodecCtx);
}