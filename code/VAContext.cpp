#include "VAContext.h"

VAContext::VAContext(const char *srcFilePath) : ContextBase(srcFilePath), videoPacketCount(0), audioPacketCount(0)
{
}
VAContext::VAContext(const char *srcFilePath, const char *dstFilepath) : VAContext(srcFilePath)
{
    // av_log_set_callback(LogCallbackTotxt);
    SetAudioOutputStreamStruct();
    SetVideoOutputStreamStruct();
    InitOutFormatContextByFileName(dstFilepath);
}
VAContext::VAContext(const char *srcFilePath, const char *dstFilepath, int fps, int width, int height) : VAContext(srcFilePath)
{
    InitOutFormatContextByFileName(dstFilepath);
    SetAudioOutputStreamStruct();
    SetVideoOutputStreamStruct(fps, width, height);
}

void VAContext::MuxeVideoAndAudio(const char *dstFilepath)
{
    if (dePacket == nullptr)
        dePacket = AllocAVPacket();
    if (outFmtCtx == nullptr)
    {
        InitOutFormatContextByFileName(dstFilepath);
    }
    // FindStreamInformation(fmtCtx);

    WriteToFilePrepare(dstFilepath);
    DecodePacket(fmtCtx);

    av_frame_free(&audioSt.frame);
    audioSt.frame = nullptr;
    EncodeAudioFrame();

    av_frame_free(&videoSt.frame);
    videoSt.frame = nullptr;
    EncodeVideoFrame();

    OutputStreamFree(audioSt);
    OutputStreamFree(videoSt);

    WriteToFile();
    WriteToFileEnd();
}

void VAContext::DealAudioPacket(AVPacket *packet)
{
    ret = avcodec_send_packet(audioSt.denc, packet);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(audioSt.denc, audioSt.deFrame);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return;

            av_log_error("Error during decoding");
            return;
        }
        /*static int audioDeFrameCount = 0;
        av_log_info("audio de frame number:%d", audioDeFrameCount++);*/

        ResampleDeAudioFrame();
        EncodeAudioFrame();
        av_frame_unref(audioSt.deFrame);
        if (ret < 0)
            return;
    }
    // audioPacketCount++;
    // audioPackets.push_back(packet);
    // av_packet_rescale_ts(packet, inAudioStream->time_base, outAudioStream->time_base);
    // packet->pos = -1;
    // av_interleaved_write_frame(outFmtCtx, packet);
}
void VAContext::DealVideoPacket(AVPacket *packet)
{
    // av_log_info("video packet pts:%d\n",packet->pts);
    ret = avcodec_send_packet(videoSt.denc, packet);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(videoSt.denc, videoSt.deFrame);
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
        av_frame_unref(videoSt.deFrame);
        if (ret < 0)
            return;
    }
    // videoPacketCount++;
    // videoPackets.push_back(packet);
    // av_packet_rescale_ts(packet, inVideoStream->time_base, outVideoStream->time_base);
    // packet->pos = -1;
    // av_interleaved_write_frame(outFmtCtx, packet);
}
void VAContext::WriteToFilePrepare(const char *dstFilepath)
{
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
    av_log_info("audio stream timebase_den:%d", outFmtCtx->streams[0]->time_base.den);
    av_log_info("video stream timebase_den:%d", outFmtCtx->streams[1]->time_base.den);
}
void VAContext::WriteToFileEnd()
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
}
void VAContext::WriteToFile()
{
    for (AVPacket *packet : audioPackets)
    {
        // av_log_info("audio packet pts:%d\n", packet->pts);
        av_interleaved_write_frame(outFmtCtx, packet);
        av_packet_free(&packet);
    }
    for (AVPacket *packet : videoPackets)
    {
        // av_log_info("video packet pts:%d\n", packet->pts);
        av_interleaved_write_frame(outFmtCtx, packet);
        av_packet_free(&packet);
    }
}
void VAContext::InitOutFormatContextByFileName(const char *dstFilePath)
{
    int audioStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    int videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVStream *audioStream = fmtCtx->streams[audioStreamIndex];
    AVStream *videoStream = fmtCtx->streams[videoStreamIndex];
    outFmtCtx = GetOutFormatContextByFileName(dstFilePath, audioSt.enc, videoSt.enc);
    // avcodec_parameters_copy(outFmtCtx->streams[audioStreamIndex]->codecpar, audioStream->codecpar);
    // avcodec_parameters_copy(outFmtCtx->streams[videoStreamIndex]->codecpar, videoStream->codecpar);
    audioSt.st = outFmtCtx->streams[0];
    videoSt.st = outFmtCtx->streams[outFmtCtx->nb_streams == 1 ? 0 : 1];
    av_dump_format(outFmtCtx, 0, dstFilePath, 1);
    inAudioStream = audioStream;
    inVideoStream = videoStream;
    outAudioStream = outFmtCtx->streams[0];
    outVideoStream = outFmtCtx->streams[1];
}
void VAContext::SetAudioOutputStreamStruct()
{
    int audioStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    AVCodecContext *audioDenc = OpenDecodecContextByStreamPar(fmtCtx->streams[audioStreamIndex]);
    audioSt.denc = audioDenc;
    AVCodecContext *audioEnc = OpenAudioEncodecContext(AV_CODEC_ID_MP3);
    audioSt.enc = audioEnc;
    // audioSt.st = outFmtCtx->streams[0];
    /*audioSt.st->codecpar->codec_tag = 0;
    audioSt.st->time_base = {1, audioEnc->sample_rate};*/
    int nb_samples;
    if (audioEnc->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = audioEnc->frame_size;
    audioSt.frame = InitAudioFrame(audioEnc, nb_samples);
    audioSt.deFrame = AllocAVFrame();
    // audioSt.packet = AllocAVPacket();
    audioSt.swr_ctx = AllocSwrContext(audioDenc, audioEnc);
    audioSt.samples_count = 0;
}
void VAContext::RescaleDeVideoFrame()
{
    ret = sws_scale(videoSt.sws_ctx, videoSt.deFrame->data, videoSt.deFrame->linesize, 0,
                    videoSt.enc->height, videoSt.frame->data, videoSt.frame->linesize);
    // av_log_info("sws scale result:%d", ret);
    if (ret <= 0)
    {
        av_log_error("sws scale the video frame failed\n");
    }
}
void VAContext::EncodeVideoFrame()
{
    if (videoSt.frame != nullptr)
    {
        videoSt.frame->pts = videoSt.next_pts;
        videoSt.next_pts += 1;
        // av_log_info("encode frame video pts:%d", videoSt.frame->pts);
    }
    ret = avcodec_send_frame(videoSt.enc, videoSt.frame);
    // av_log_info("send frame ret:%d", ret);
    if (ret < 0)
    {
        // av_log_error("send frame to enVideoCodecContext error\n");
        return;
    }
    while (ret >= 0)
    {
        videoSt.packet = AllocAVPacket();
        ret = avcodec_receive_packet(videoSt.enc, videoSt.packet);
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
        // av_log_info("write a packet\n");
        av_packet_rescale_ts(videoSt.packet, videoSt.enc->time_base, outVideoStream->time_base);
        /*AVRational temp = { 1,AV_TIME_BASE };
        double duration = (double)1 / av_q2d(inVideoStream->r_frame_rate);
        videoSt.packet->pts = (double)(videoSt.next_pts * duration) / av_q2d(temp);
        videoSt.packet->pts = av_rescale_q_rnd(videoSt.packet->pts,temp,outVideoStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        videoSt.packet->dts = videoSt.packet->pts;
        videoSt.next_pts++;*/
        // videoSt.packet->pos = -1;
        videoSt.packet->stream_index = videoSt.st->index;
        videoPackets.push_back(videoSt.packet);
        // av_interleaved_write_frame(outFmtCtx, videoSt.packet);
        //  avio_flush(outFmtCtx->pb);
        //  av_log_info("flush a packet\n");
        // av_packet_unref(videoSt.packet);
        if (ret < 0)
            return;
    }
}
void VAContext::ResampleDeAudioFrame()
{
    int64_t delay = swr_get_delay(audioSt.swr_ctx, audioSt.deFrame->sample_rate);
    int64_t dstNbSamples = av_rescale_rnd(delay + audioSt.deFrame->nb_samples, audioSt.enc->sample_rate,
                                          audioSt.deFrame->sample_rate, AV_ROUND_UP);
    if (dstNbSamples > maxDstNbSamples)
    {
        av_frame_free(&(audioSt.frame));
        audioSt.frame = InitAudioFrame(audioSt.enc, dstNbSamples);
        maxDstNbSamples = dstNbSamples;
    }
    ret = swr_convert(audioSt.swr_ctx, audioSt.frame->data, dstNbSamples,
                      const_cast<const uint8_t **>(audioSt.deFrame->data), audioSt.deFrame->nb_samples);
    if (ret < 0)
    {
        av_log_error("resample is failed\n");
        return;
    }
}
void VAContext::EncodeAudioFrame()
{
    if (audioSt.frame != nullptr)
    {
        audioSt.frame->pts = audioSt.samples_count;
        audioSt.samples_count += audioSt.frame->nb_samples;
    }
    ret = avcodec_send_frame(audioSt.enc, audioSt.frame);
    if (ret < 0)
        return;
    /*static int audioEnFrame = 0;
    av_log_info("audio en frame number:%d",audioEnFrame++);*/
    while (ret >= 0)
    {
        audioSt.packet = AllocAVPacket();
        ret = avcodec_receive_packet(audioSt.enc, audioSt.packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            av_log_error("Error encoding audio frame");
            return;
        }
        // av_packet_rescale_ts(audioSt.packet, audioSt.enc->time_base, outAudioStream->time_base);
        // AVRational temp = { 1,AV_TIME_BASE };
        // double duration = (double)1 /outAudioStream->time_base.den;
        // audioSt.packet->pts = (double)(audioSt.next_pts * duration) / av_q2d(temp);
        // audioSt.packet->pts = av_rescale_q_rnd(audioSt.packet->pts, temp, outAudioStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        // audioSt.packet->dts = audioSt.packet->pts;
        // audioSt.next_pts++;
        audioSt.packet->stream_index = audioSt.st->index;
        // audioSt.packet->pos = -1;
        //  av_interleaved_write_frame(outFmtCtx, audioSt.packet);
        audioPackets.push_back(audioSt.packet);
        // av_log_info("push back a audio packet:%d", audioPacketCount++);
        // av_packet_unref(audioSt.packet);
    }
}
void VAContext::OutputStreamFree(OutputStream &ost)
{
    avcodec_free_context(&ost.enc);

    av_frame_free(&ost.frame);
    av_frame_free(&ost.deFrame);
    av_packet_free(&ost.packet);
    sws_freeContext(ost.sws_ctx);
    swr_free(&ost.swr_ctx);
}
void VAContext::SetVideoOutputStreamStruct()
{
    int videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVCodecContext *videoDenc = OpenDecodecContextByStreamPar(fmtCtx->streams[videoStreamIndex]);
    videoSt.denc = videoDenc;
    videoSt.enc = OpenVideoEncodecContext(AV_CODEC_ID_H264, 25, videoDenc->width, videoDenc->height); // error prone
    videoSt.frame = InitVideoAVFrame(videoSt.enc);
    videoSt.deFrame = AllocAVFrame();
    // videoSt.st = outFmtCtx->streams[outFmtCtx->nb_streams == 1 ? 0 : 1];
    // videoSt.st->id = 1;
    // videoSt.st->codecpar->codec_tag = 0;
    //  videoSt.packet = AllocAVPacket();
    videoSt.sws_ctx = AllocSwsContext(videoDenc->pix_fmt, videoDenc->width, videoDenc->height, videoSt.enc);
    videoSt.next_pts = 0;
}
void VAContext::SetVideoOutputStreamStruct(int fps, int width, int height)
{
    int videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVCodecContext *videoDenc = OpenDecodecContextByStreamPar(fmtCtx->streams[videoStreamIndex]);
    videoSt.denc = videoDenc;
    videoSt.enc = OpenVideoEncodecContext(AV_CODEC_ID_H264, fps, width, height);
    videoSt.frame = InitVideoAVFrame(videoSt.enc);
    videoSt.deFrame = AllocAVFrame();
    // videoSt.st = outFmtCtx->streams[outFmtCtx->nb_streams == 1 ? 0 : 1];
    videoSt.st->id = 1;
    videoSt.st->codecpar->codec_tag = 0;
    // videoSt.packet = AllocAVPacket();
    videoSt.sws_ctx = AllocSwsContext(videoDenc->pix_fmt, videoDenc->width, videoDenc->height, videoSt.enc);
    videoSt.next_pts = 0;
}