#include "AVFileUnityTest.h"

void AVFileUnityTest::DealAudioPacket()
{
    if (isOtherStream == true)
        return;
    AudioContext::ret = avcodec_send_packet(deAudioCodecCtx, dePacket);
    if (AudioContext::ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return;
    }
    while (AudioContext::ret >= 0)
    {
        AudioContext::ret = avcodec_receive_frame(deAudioCodecCtx, deAudioFrame);
        if (AudioContext::ret < 0)
        {
            if (AudioContext::ret == AVERROR_EOF || AudioContext::ret == AVERROR(EAGAIN))
                return;
            av_log_error("Error during decoding");
            return;
        }
        av_log_info("receive a deaudio frame\n");
        avContext->WriteAudioToFile(deAudioFrame);
        av_frame_unref(deAudioFrame);
        if (AudioContext::ret < 0)
            return;
    }
}
void AVFileUnityTest::DealVideoPacket()
{
    if (isOtherStream == false)
        return;
    ret = avcodec_send_packet(deVideoCodecCtx, dePacket);
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
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return;
            av_log_error("Error during decoding");
            return;
        }
        av_log_info("receive a devideo frame\n");
        avContext->WriteVideoToFile(deVideoFrame);
        av_frame_unref(deVideoFrame);
        if (ret < 0)
            return;
    }
}
AVFileUnityTest::AVFileUnityTest() : AVFileContext()
{
    isOtherStream = true;
}
void AVFileUnityTest::Test(const char *srcFilePath, const char *dstFilePath)
{
    VideoFileContext::VariableCheck(srcFilePath);
    AudioFileContext::VariableCheck(srcFilePath);
    AVStream *audioStream = inFmtCtx->streams[getAudioStreamIndex()];
    AVStream *videoStream = inFmtCtx->streams[getVideoStreamIndex()];
    avContext = new AVContext(dstFilePath, audioStream->codecpar->sample_rate, (AVSampleFormat)audioStream->codecpar->format, audioStream->codecpar->ch_layout, (AVPixelFormat)videoStream->codecpar->format, videoStream->r_frame_rate.num, 0.2f,videoStream->codecpar->width, videoStream->codecpar->height);
    avContext->WriteAVPreparition(dstFilePath);
    isOtherStream = true;
    DecodePakcet();
    isOtherStream = false;
    av_seek_frame(inFmtCtx,getAudioStreamIndex(),0,0);
    DecodePakcet();
    avContext->WriteAVTailer();
}
AVFileUnityTest::~AVFileUnityTest() {
    delete avContext;
}