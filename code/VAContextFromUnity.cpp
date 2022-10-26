#include "VAContextFromUnity.h"

VAContextFromUnity::VAContextFromUnity(const char *srcFilePath, const char *dstFilePath) : ContextBase(srcFilePath), AudioContext(srcFilePath),
                                                                                           VideoContext(srcFilePath), isAudio(true),
                                                                                           outFmtCtx(GetOutFormatContextByFileName(dstFilePath, enAudioCodecCtx, enVideoCodecCtx))
{
    outAudioStream = outFmtCtx->streams[0];
    outVideoStream = outFmtCtx->streams[1];
    deAudioFrame = AllocAVFrame();
    deVideoFrame = AllocAVFrame();
    // av_dump_format(outFmtCtx,0,dstFilePath,0);
}
VAContextFromUnity::VAContextFromUnity(const char *dstFilePath,
                                       int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout,
                                       AVPixelFormat dePixFormat, int fps, int deWidth, int deHeight) : ContextBase(), AudioContext(sampleRate, sampleFormat, chLayout),
                                                                                                        VideoContext(dePixFormat, fps, deWidth, deHeight),
                                                                                                        outFmtCtx(GetOutFormatContextByFileName(dstFilePath, enAudioCodecCtx, enVideoCodecCtx))
{
    outAudioStream = outFmtCtx->streams[0];
    outVideoStream = outFmtCtx->streams[1];
    deAudioFrame = AllocAVFrame();
    deVideoFrame = AllocAVFrame();
    deAudioFrame->sample_rate = sampleRate;
    deAudioFrame->format = sampleFormat;
    deVideoFrame->linesize[0] = deWidth * 4;
}
void VAContextFromUnity::MuexFileToMp4File(const char *srcFilePath, const char *dstFilePath)
{
    if (dePacket == nullptr)
        dePacket = AllocAVPacket();
    WriteToFilePrepare(dstFilePath);
    DecodePacket(fmtCtx);
    av_frame_free(&enAudioFrame);
    enAudioFrame = nullptr;
    EncodeAudioFrame();
    isAudio = false;
    ReSetAVFormatContext(srcFilePath);
    DecodePacket(fmtCtx);
    av_frame_free(&enVideoFrame);
    enVideoFrame = nullptr;
    EncodeVideoFrame();
    WriteToFileEnd();
}

void VAContextFromUnity::WriteToFilePrepare(const char *dstFilepath)
{

    // open target file stream
    ret = avio_open(&outFmtCtx->pb, dstFilepath, AVIO_FLAG_WRITE);
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
void VAContextFromUnity::WriteToFileEnd()
{
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
    avformat_free_context(outFmtCtx);
    outFmtCtx = nullptr;
    av_frame_free(&deAudioFrame);
    av_frame_free(&deVideoFrame);
    AudioContext::pts = 0;
    VideoContext::pts = 0;
}
void VAContextFromUnity::DealAudioPacket(AVPacket *packet)
{
    if (isAudio == false)
        return;
    AudioContext::DealAudioPacket(packet);
}
void VAContextFromUnity::DealVideoPacket(AVPacket *packet)
{
    if (isAudio == true)
        return;
    VideoContext::DealVideoPacket(packet);
}
void VAContextFromUnity::EncodeAudioFrame()
{
    if (enAudioFrame != nullptr)
    {
        enAudioFrame->pts = AudioContext::pts;
        AudioContext::pts += enAudioFrame->nb_samples;
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
        av_interleaved_write_frame(outFmtCtx, enAudioPacket);
        av_packet_unref(enAudioPacket);
        if (ret < 0)
            return;
    }
}

void VAContextFromUnity::EncodeVideoFrame()
{
    if (enVideoFrame != nullptr)
    {
        enVideoFrame->pts = VideoContext::pts;
        VideoContext::pts += 1;
        av_log_info("encode frame video pts:%d", enVideoFrame->pts);
    }
    ret = avcodec_send_frame(enVideoCodecCtx, enVideoFrame);
    av_log_info("send frame ret:%d", ret);
    if (ret < 0)
    {
        // av_log_error("send frame to enVideoCodecContext error\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enVideoCodecCtx, enVideoPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            // av_log_error("eagain averror_eof %d\n", ret);
            return;
        }

        if (ret < 0)
        {
            av_log_error("enVideoCodecContext receive packet error\n");
            return;
        }
        av_log_info("packet origin pts:%d,envideocontext timebase_den:%d,outvideostream timebase_den:%d", enVideoPacket->pts, enVideoCodecCtx->time_base.den, outVideoStream->time_base.den);
        av_packet_rescale_ts(enVideoPacket, enVideoCodecCtx->time_base, outVideoStream->time_base);
        av_log_info("packet rescale pts:%d", enVideoPacket->pts);
        enVideoPacket->stream_index = outVideoStream->index;
        av_interleaved_write_frame(outFmtCtx, enVideoPacket);
        av_packet_unref(enVideoPacket);
        if (ret < 0)
            return;
    }
}

void VAContextFromUnity::WriteAudioFrame(void *data, int length)
{
    deAudioFrame->data[0] = (uint8_t *)data;
    deAudioFrame->data[1] = (uint8_t *)((float *)data + length);
    deAudioFrame->nb_samples = length;
    // av_log_info("deAudioFrame sampleRate:%d,nbSamples:%d,sampleFmt:%d\n", deAudioFrame->sample_rate, deAudioFrame->nb_samples,deAudioFrame->format);
    // deAudioFrame->linesize[0] = length * 8;
    ResampleDeAudioFrame();
    // enAudioFrame->data[0] = (uint8_t*)data;
    // enAudioFrame->data[1] = (uint8_t*)((float*)data + length);
    // enAudioFrame->nb_samples = length;
    EncodeAudioFrame();
}
void VAContextFromUnity::WriteVideoFrame(void *data)
{
    deVideoFrame->data[0] = (uint8_t *)data;
    // deVideoFrame->linesize[0] = deWidth * 4;
    RescaleDeVideoFrame();
    EncodeVideoFrame();
}