#include "AVFileContext.h"

AVFileContext::AVFileContext() : FileContextBase(), AudioFileContext(), VideoFileContext()
{
}
AVFileContext::AVFileContext(const char *srcFilePath) : FileContextBase(srcFilePath), AudioFileContext(), VideoFileContext() {}
AVFileContext::AVFileContext(int fps, int width, int height) : FileContextBase(), AudioFileContext(), VideoFileContext(fps, width, height), ret(0)
{
}
AVFileContext::AVFileContext(int deWidth, int deHeight, int fps, int width, int height) : FileContextBase(), AudioFileContext(), VideoFileContext(deWidth, deHeight, fps, width, height), ret(0)
{
}
AVFileContext::AVFileContext(AVCodecID audioDstCodecID, AVPixelFormat srcPixFormat, AVCodecID codecID, int deWidth, int deHeight, int fps, int width, int height) : FileContextBase(), AudioFileContext(audioDstCodecID), VideoFileContext(srcPixFormat, codecID, deWidth, deHeight, fps, width, height), ret(0)
{
}
bool AVFileContext::ExtractAVToFile(const char *srcFilePath, const char *dstFilePath)
{
    if (VideoFileContext::VariableCheck(srcFilePath) == false)
        return false;
    if (AudioFileContext::VariableCheck(srcFilePath) == false)
        return false;
    WriteAVPreparition(dstFilePath);
    DecodePakcet();
    WriteAVTailer();
    return true;
}
void AVFileContext::WriteAVPreparition(const char *dstFilePath)
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
void AVFileContext::WriteAVTailer()
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

    VideoFileContext::pts = 0;
    AudioFileContext::pts = 0;
}
AVFileContext::~AVFileContext()
{
    avformat_free_context(outAVFmtCtx);
}