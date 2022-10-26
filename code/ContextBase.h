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
#include <utility>
#include <exception>
//#define av_log_error(str, ...) av_log(nullptr, AV_LOG_ERROR, str, __VA_ARGS__)
//#define av_log_info(str, ...) av_log(nullptr, AV_LOG_INFO, str, __VA_ARGS__)
#define av_log_error(format, ...) Debug::LogError(format, ##__VA_ARGS__);
#define av_log_info(format, ...) Debug::Log(format, ##__VA_ARGS__);
// #define av_log_error(str, ...)
// #define av_log_info(str, ...)
void UnityLogCallbackFunc(void *, int, const char *, va_list);
void LogCallbackTotxt(void *, int, const char *, va_list);
typedef void (*LogFunc)(char *message);
struct OutputStream
{
    AVStream *st;
    AVCodecContext *enc;
    AVCodecContext *denc;
    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;
    AVFrame *frame;
    AVFrame *deFrame;
    AVPacket *packet;
    SwsContext *sws_ctx;
    SwrContext *swr_ctx;
};
class ContextBase
{
protected:
    AVFormatContext *fmtCtx;

    AVPacket *dePacket;

protected:
    AVFormatContext *GetFormatContextByFileName(const char *fileName);
    AVFormatContext *GetOutFormatContextByFileName(const char *dstFileName, const AVCodecContext *enAudioCodecCtx, const AVCodecContext *enVideoCodecCtx);
    void FindStreamInformation(AVFormatContext *formatContext);
    AVCodecContext *OpenVideoEncodecContext(AVCodecID codecId, int fps, int width, int height);
    SwsContext *AllocSwsContext(AVPixelFormat dePixFormat, int width, int height, AVCodecContext *enCodecCtx);
    AVFrame *InitVideoAVFrame(AVCodecContext *codecContxt);
    AVCodecContext *OpenAudioEncodecContext(AVCodecID codecId);
    SwrContext *AllocSwrContext(AVChannelLayout layout, int sampleRate, AVSampleFormat sampleFmt, AVCodecContext *enCodecCtx);
    SwrContext *AllocSwrContext(AVCodecContext *deCodecCtx, AVCodecContext *enCodecCtx);
    void InitAudioFrame(AVFrame *&frame, AVCodecContext *codecCtx, int dstNbSamples);
    AVFrame *InitAudioFrame(AVCodecContext *codecCtx, int dstNbSamples);
    AVFrame *AllocAVFrame();
    AVPacket *AllocAVPacket();
    AVCodecContext *OpenDecodecContextByStreamPar(AVStream *stream);
    AVCodecContext *AllocDecodecContext(AVCodecID codecId);
    AVCodecContext *AllocEnCodecContext(AVCodecID codecId);
    void DecodePacket(AVFormatContext *formatContext);
    virtual void DealAudioPacket(AVPacket *packet);
    virtual void DealVideoPacket(AVPacket *packet);
    std::pair<int, int> FindAVStreamIndex() const;

public:
    int ret;
    ContextBase();
    ContextBase(const char *srcFilePath);
    void ReSetAVFormatContext(const char *srcFilePath);
    void DumpFileInformation(const char *fileName) const;
    const AVFormatContext *GetFormatContext() const;
    virtual ~ContextBase();
};

class Debug
{
public:
    static void (*LogFunPtr)(char *message, int size);
    static void (*LogErrorFunPtr)(char *message, int size);
    static void Log(const char *msg, ...);
    static void LogError(const char *msg, ...);
};