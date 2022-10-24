#include "AVContext.h"
AVContext::AVContext() : outAVFmtCtx(nullptr), ret(0), VideoContext(), AudioContext() {}
AVContext::AVContext(const char *dstFilePath,
                     int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout,
                     AVPixelFormat dePixFormat, int fps, float bitRatePercent,int width, int height) : VideoContext(dePixFormat, DEFAULTVIDEOCODECID,fps, bitRatePercent,width, height), AudioContext(sampleRate, sampleFormat, chLayout), ret(0)
{
    outAVFmtCtx = AllocOutFormatContext(dstFilePath);
    if (outAVFmtCtx == nullptr)
    {
        ret = -1;
        av_log_error("alloc outformat context failed\n");
        return;
    }
    outVideoFmtCtx = outAudioFmtCtx = outAVFmtCtx;
    AddNewStreamToFormat(outAVFmtCtx, enVideoCodecCtx);
    outVideoStream = outAVFmtCtx->streams[outAVFmtCtx->nb_streams - 1];
    AddNewStreamToFormat(outAVFmtCtx, enAudioCodecCtx);
    outAudioStream = outAVFmtCtx->streams[outAVFmtCtx->nb_streams - 1];
}
AVContext::AVContext(const char *dstFilePath,
                     int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout,
                     AVPixelFormat dePixFormat, int fps, float bitRatePercent,int deWidth, int deHeight, int enWidth, int enHeight):
    VideoContext(dePixFormat, DEFAULTVIDEOCODECID, fps, bitRatePercent,deWidth, deHeight,enWidth,enHeight), AudioContext(sampleRate, sampleFormat, chLayout), ret(0) {}
void AVContext::WriteAVPreparition(const char *dstFilePath)
{
    if (VideoContext::VariableCheck() == false || AudioContext::VariableCheck() == false)
    {
        av_log_error("variable check failed\n");
        return;
    }
    if (outAVFmtCtx == nullptr) {
        outAVFmtCtx = AllocOutFormatContext(dstFilePath);
        if (outAVFmtCtx == nullptr)
        {
            ret = -1;
            av_log_error("alloc outformat context failed\n");
            return;
        }
        outVideoFmtCtx = outAudioFmtCtx = outAVFmtCtx;
        AddNewStreamToFormat(outAVFmtCtx, enVideoCodecCtx);
        outVideoStream = outAVFmtCtx->streams[outAVFmtCtx->nb_streams - 1];
        AddNewStreamToFormat(outAVFmtCtx, enAudioCodecCtx);
        outAudioStream = outAVFmtCtx->streams[outAVFmtCtx->nb_streams - 1];
    }
    // open target file stream
    ret = avio_open(&outAVFmtCtx->pb, dstFilePath, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        av_log_error("avio open failed\n");
        return;
    }
    // write file header information
    ret = avformat_write_header(outAVFmtCtx, nullptr);
    if (ret < 0)
    {
        av_log_error("write header information failed\n");
        return;
    }
}
void AVContext::WriteAVTailer()
{
    ret = av_write_trailer(outAVFmtCtx);
    if (ret < 0)
    {
        av_log_error("write trailer information failed\n");
        return;
    }
    ret = avio_close(outAVFmtCtx->pb);
    if (ret < 0)
    {
        av_log_error("close outfmt context failed\n");
        return;
    }
    avformat_free_context(outAVFmtCtx);
    outAVFmtCtx = outVideoFmtCtx = outAudioFmtCtx = nullptr;
    outVideoStream = outAudioStream = nullptr;

    av_frame_free(&deVideoFrame);
    deVideoFrame = nullptr;
    av_frame_free(&deAudioFrame);
    deAudioFrame = nullptr;

    VideoContext::pts = 0;
    AudioContext::pts = 0;
}

int AVContext::GetRet() const
{
    if (VideoContext::ret < 0) return VideoContext::ret;
    if (AudioContext::ret < 0) return AudioContext::ret;
    return ret;
}
