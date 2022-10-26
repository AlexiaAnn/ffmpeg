#include "MP42MP3Context.h"
void FFmpegLogCallbackFunc(void *ptr, int level, const char *fmt, va_list vl)
{
    char printf_buf[1024];
    vsprintf(printf_buf, fmt, vl);
    Debug::Log(printf_buf);
}
MP42MP3Context::MP42MP3Context(const char *srcFileStr, const char *dstFileStr) : srcFileName(srcFileStr),
                                                                                 dstFileName(dstFileStr),
                                                                                 dstFrame(nullptr),
                                                                                 max_dst_nb_samples(0)
{
    logger = nullptr;
    swrCtx = nullptr;
    // av_log_set_callback(MyLogOutput);
    dstFilePtr = fopen(dstFileName, "wb");
    formatContext = GetFormatContextByFileName(srcFileName, logger);
    Find_stream_info(formatContext, logger);
    audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    AVStream *audioStream = formatContext->streams[audioStreamIndex];
    Open_decodec_context(audioStream, &deCdcCtx, formatContext, AVMEDIA_TYPE_AUDIO, logger);
    Open_encodec_context(audioStream, AV_CODEC_ID_MP3, &enCdcCtx, logger);
    srcFrame = av_frame_alloc();
    dePacket = av_packet_alloc();
    enPacket = av_packet_alloc();
}
MP42MP3Context::~MP42MP3Context()
{
    swr_free(&swrCtx);
    avcodec_free_context(&deCdcCtx);
    avcodec_free_context(&enCdcCtx);
    av_packet_free(&dePacket);
    av_packet_free(&enPacket);
    avformat_close_input(&formatContext);
    int result = fclose(dstFilePtr);
    if (result == EOF)
    {
        av_log_info("文件流关闭失败");
    }
    else
    {
        av_log_info("文件流关闭成功");
    }
}

void MP42MP3Context::Mp42Pcm()
{
    TempDecodePacket(&MP42MP3Context::EncodePcm);
}
void MP42MP3Context::Mp42Mp3()
{
    SetSwrContext();
    DecodePacket(&MP42MP3Context::ResampleFrame);
}

int MP42MP3Context::SetSwrContext()
{
    //申请内存
    swrCtx = swr_alloc();
    if (!swrCtx)
    {
        av_log_error("could not allocate resampler context");
        return AVERROR(ENOMEM);
    }
    //设置重采样参数
    av_opt_set_chlayout(swrCtx, "in_chlayout", &(deCdcCtx->ch_layout), 0);
    av_opt_set_int(swrCtx, "in_sample_rate", deCdcCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", deCdcCtx->sample_fmt, 0);
    av_opt_set_chlayout(swrCtx, "out_chlayout", &(enCdcCtx->ch_layout), 0);
    av_opt_set_int(swrCtx, "out_sample_rate", enCdcCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", enCdcCtx->sample_fmt, 0);
    //初始化重采样上下文
    int ret = 0;
    if ((ret = swr_init(swrCtx)) < 0)
    {
        av_log_error("failed to initialize the resampling context");
        return ret;
    }
    max_dst_nb_samples = av_rescale_rnd(deCdcCtx->frame_size, deCdcCtx->sample_rate, enCdcCtx->sample_rate, AV_ROUND_UP);
    return ret;
}
void MP42MP3Context::InitDstFrame(int64_t dst_nb_samples)
{
    dstFrame = av_frame_alloc();
    dstFrame->sample_rate = enCdcCtx->sample_rate;
    dstFrame->format = enCdcCtx->sample_fmt;
    dstFrame->ch_layout = enCdcCtx->ch_layout;
    dstFrame->nb_samples = dst_nb_samples;
    //分配buffer
    if (av_frame_get_buffer(dstFrame, 0) < 0)
    {
        av_log_error("frame get buffer is failed");
        return;
    }
    if (av_frame_make_writable(dstFrame) < 0)
    {
        av_log_error("frame is not writeable");
        return;
    }
}
void MP42MP3Context::ResetDstFrame(int64_t dst_nb_samples)
{
    av_frame_free(&dstFrame);
    InitDstFrame(dst_nb_samples);
}

void MP42MP3Context::TempDecodePacket(void (MP42MP3Context::*funcP)())
{
    static int packetCount = 0;
    while (av_read_frame(formatContext, dePacket) >= 0)
    {
        if (dePacket->stream_index == audioStreamIndex)
        {
            packetCount++;
            ReciveFrame(funcP);
        }
        av_packet_unref(dePacket);
    }

    av_log_info("packet numbers:{0}");
    const char *fmt;
    get_format_from_sample_fmt(&fmt, enCdcCtx->sample_fmt);
    printf("Play the output audio file with the command:"
           "ffplay -f %s -ac %d -ar %d %s\n",
           fmt, enCdcCtx->ch_layout.nb_channels, enCdcCtx->sample_rate,
           dstFileName);
}
void MP42MP3Context::DecodePacket(void (MP42MP3Context::*funcP)())
{
    while (av_read_frame(formatContext, dePacket) >= 0)
    {
        dePacketCount++;
        if (dePacket->stream_index == audioStreamIndex)
        {
            deAPacketCount++;
            (this->*funcP)();
        }
        av_packet_unref(dePacket); //引用基数减1？？？？
    }
    av_log_info("send all packet over\n");
    // av_log_info("dePacket number:%d\n", dePacketCount);
    // av_log_info("deAudioPacket number:%d\n", deAPacketCount);
    // av_log_info("deFrame number:%d\n", deFrameCount);
    // av_log_info("enFrame number:%d\n", enFrameCount);
    // av_log_info("enPacket number:%d\n", enPacketCount);
    av_log_info("send all packet over,packet numbers:%d,audio packet numbers:%d,video packet numbers:%d\n", dePacketCount, deAPacketCount, dePacketCount - deAPacketCount);
    /*const char* fmt;
    get_format_from_sample_fmt(&fmt, enCdcCtx->sample_fmt);
    printf("Play the output audio file with the command:\n"
        "ffplay -f %s -ac %d -ar %d %s\n",
        fmt, enCdcCtx->ch_layout.nb_channels, enCdcCtx->sample_rate,
        dstFileName);*/
}
int MP42MP3Context::ReciveFrame(void (MP42MP3Context::*funcP)())
{
    int ret = avcodec_send_packet(deCdcCtx, dePacket);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding");
        return ret;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(deCdcCtx, srcFrame);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            av_log_error("Error during decoding\n");
            return ret;
        }
        static int audio_frame_count = 0;
        // av_log_info("audio_frame n:{0} nb_samples:{1} pts:", audio_frame_count++, srcFrame->nb_samples);
        av_log_info("audio_frame n:{0} nb_samples:{1} pts:");
        (this->*funcP)();
        av_frame_unref(srcFrame);
        if (ret < 0)
            return ret;
    }
    return 0;
}
void MP42MP3Context::ResampleFrame()
{
    int ret = avcodec_send_packet(deCdcCtx, dePacket);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(deCdcCtx, srcFrame);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return;

            av_log_error("Error during decoding");
            return;
        }
        static int audio_frame_count = 0;
        // av_log_info("audio_frame n:{0} nb_samples:{1} pts:", audio_frame_count++, srcFrame->nb_samples);

        Resample();
        deFrameCount++;
        av_frame_unref(srcFrame);

        if (ret < 0)
            return;
    }
}
void MP42MP3Context::Resample()
{
    // write the frame data to output file
    int src_nb_samples = srcFrame->nb_samples;
    int64_t delay = swr_get_delay(swrCtx, srcFrame->sample_rate);
    int64_t dst_nb_samples = av_rescale_rnd(delay + srcFrame->nb_samples, enCdcCtx->sample_rate, srcFrame->sample_rate, AV_ROUND_UP);
    if (dstFrame == nullptr)
        InitDstFrame(dst_nb_samples);
    if (dst_nb_samples > max_dst_nb_samples)
    {
        // av_log_info("dst_nb_samples > max_dst_nb_samples occured,reapply frame buffer");
        ResetDstFrame(dst_nb_samples);
        max_dst_nb_samples = dst_nb_samples;
    }
    int ret = swr_convert(swrCtx, dstFrame->data, dst_nb_samples, const_cast<const uint8_t **>(srcFrame->data), srcFrame->nb_samples);
    if (ret < 0)
    {
        av_log_error("resample is failed");
    }
    else
    {
        int data_size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(dstFrame->format));
        // av_log_info("重采样成功：%d----dst_nb_samples:%d---data_size:%d\n", ret, dst_nb_samples, data_size);
        // av_log_info("重采样成功：----dst_nb_samples:---data_size:\n");
        // std::cout << "重采样成功：" << ret << "----dst_nb_samples:" << dst_nb_samples << "---data_size:" << data_size << std::endl;
        ////for (int i = 0; i < ret; i++) {
        ////	for (int ch = 0; ch < dstFrame->ch_layout.nb_channels; ch++) {
        ////		// 需要储存为pack模式
        ////		fwrite(dstFrame->data[ch] + data_size * i, 1, data_size, dstFilePtr);
        ////	}
        ////}
        int ret = avcodec_send_frame(enCdcCtx, dstFrame);
        enFrameCount++;
        while (ret >= 0)
        {
            ret = avcodec_receive_packet(enCdcCtx, enPacket);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return;
            else if (ret < 0)
            {
                av_log_error("Error encoding audio frame");
                exit(1);
            }
            // av_log_info("enpacket count:%d",aPacketCount);
            fwrite(enPacket->data, 1, enPacket->size, dstFilePtr);
            enPacketCount++;
            av_packet_unref(enPacket);
        }
    }
}
void MP42MP3Context::EncodePcm()
{
    // write the frame data to output file
    int data_size = av_get_bytes_per_sample(deCdcCtx->sample_fmt);
    if (data_size < 0)
    {
        av_log_error("Error during deconding");
        return;
    }
    for (int i = 0; i < srcFrame->nb_samples; i++)
        for (int ch = 0; ch < deCdcCtx->ch_layout.nb_channels; ch++)
            fwrite(srcFrame->data[ch] + data_size * i, 1, data_size, dstFilePtr);
}