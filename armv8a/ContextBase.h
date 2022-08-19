#pragma once
#define __STDC_CONSTANT_MACROS
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libavutil/imgutils.h>
}
//#define av_log_error(str, ...) av_log(nullptr, AV_LOG_ERROR, str, __VA_ARGS__)
//#define av_log_info(str, ...) av_log(nullptr, AV_LOG_INFO, str, __VA_ARGS__)
// #define av_log_error(str, ...) Debug::Log(str, ##__VA_ARGS__);
// #define av_log_info(str, ...) Debug::Log(str, ##__VA_ARGS__);
#define av_log_error(str, ...)
#define av_log_info(str, ...)
void UnityLogCallbackFunc(void *, int, const char *, va_list);
void LogCallbackTotxt(void *, int, const char *, va_list);
typedef void (*LogFunc)(char *message);
class ContextBase
{
protected:
    AVFormatContext *fmtCtx;

    AVPacket *dePacket;

protected:
    AVFormatContext *GetFormatContextByFileName(const char *fileName);
    AVFormatContext *GetOutFormatContextByFileName(const char *dstFileName, AVCodecContext *enAudioCodecCtx, AVCodecContext *enVideoCodecCtx);
    void FindStreamInformation(AVFormatContext *formatContext);
    AVFrame *AllocAVFrame();
    AVPacket *AllocAVPacket();
    AVCodecContext *OpenDecodecContextByStreamPar(AVStream *stream);
    AVCodecContext *AllocDecodecContext(AVCodecID codecId);
    AVCodecContext *AllocEnCodecContext(AVCodecID codecId);
    void DecodePacket(AVFormatContext *formatContext);
    virtual void DealAudioPacket(AVPacket *packet);
    virtual void DealVideoPacket(AVPacket *packet);

public:
    int ret;
    ContextBase();
    ContextBase(const char *srcFilePath);
    void ReSetAVFormatContext(const char *srcFilePath);
    void DumpFileInformation(const char *fileName) const;
    virtual ~ContextBase();
};

class Debug
{
public:
    static void (*LogFunPtr)(char *message, int size);
    static void Log(const char *msg, ...);
};