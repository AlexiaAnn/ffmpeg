#include "AudioFileContext.h"

void AudioFileContext::DealAudioPacket()
{
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
        //av_log_info("receive a deaudio frame\n");
        DealDeAudioFrame();
        av_frame_unref(deAudioFrame);
        if (AudioContext::ret < 0)
            return;
    }
}
void AudioFileContext::DealDeAudioFrame()
{
    ResampleDeAudioFrame();
    EncodeAudioFrame();
}
bool AudioFileContext::VariableCheck(const char *srcFilePath)
{
    if (FileContextBase::ret < 0 || AudioContext::ret < 0)
    {
        av_log_error("construct error\n");
        return false;
    }
    ResetFormatContextByFileName(srcFilePath);
    if (dePacket == nullptr)
        dePacket = AllocAVPacket();
    AVStream *audioStream = inFmtCtx->streams[getAudioStreamIndex()];
    if (deAudioCodecCtx == nullptr)
    {

        deAudioCodecCtx = OpenDecodecContextByStream(audioStream);
        av_log_info("deCodecContext is nullptr,now open the deCodecContext\n");
    }
    srcSampleFormat = deAudioCodecCtx->sample_fmt;
    srcSampleRate = deAudioCodecCtx->sample_rate;
    srcChlayout = deAudioCodecCtx->ch_layout;
    return AudioContext::VariableCheck();
}
AVFrame* AudioFileContext::GetNextAudioFrame()
{
    //首先从解码器中解码已经缓存的packet数据
    ret = avcodec_receive_frame(deAudioCodecCtx, deAudioFrame);
    if (ret >= 0)
        goto the_end;
    //没有已缓存的，需要重新取packet
retry:
    do
    {
        ret = av_read_frame(inFmtCtx, dePacket);
        if (ret < 0)
            return nullptr; //没有packet，已经全部读取完成
    } while (dePacket->stream_index != getAudioStreamIndex());

    //还有packet，从中获取新的frame
    ret = avcodec_send_packet(deAudioCodecCtx, dePacket);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return nullptr;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(deAudioCodecCtx, deAudioFrame);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                goto retry;
            av_log_error("Error during decoding");
            break;
        }
        break; // error prone
        if (ret < 0)
            break;
    }
the_end:
    return deAudioFrame;
}
AudioFileContext::AudioFileContext() : FileContextBase(), AudioContext(), ret(0), deAudioCodecCtx(nullptr)
{
}
AudioFileContext::AudioFileContext(AVCodecID codecId):FileContextBase(),AudioContext(codecId),ret(0),deAudioCodecCtx(nullptr)
{
}
bool AudioFileContext::ExtractAudioToFile(const char *srcFilePath, const char *dstFilePath)
{
    av_log_info("extract audio file start\n");
    VariableCheck(srcFilePath);
    WriteAudioPreparition(dstFilePath);
    DecodePakcet();
    enAudioFrame = nullptr;
    EncodeAudioFrame();
    WriteAudioTrailer();
    av_log_info("extract audio file end\n");
    return AudioContext::ret >= 0;
}