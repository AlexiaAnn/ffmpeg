#pragma once
#define __STDC_CONSTANT_MACROS
//#define CmdLog
#define UnityLog
//#define NoneLog
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
#include <libavutil/time.h>
#include <libavutil/fifo.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
    // #include "cmdutils.h"
}
#include <exception>
#include <thread>
#include <iostream>
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#ifdef CmdLog
#define av_log_error(str, ...) av_log(nullptr, AV_LOG_ERROR, str, ##__VA_ARGS__)
#define av_log_info(str, ...) av_log(nullptr, AV_LOG_INFO, str, ##__VA_ARGS__)
#define av_log_warning(str, ...) av_log(nullptr, AV_LOG_WARNING, str, ##__VA_ARGS__)
#define av_log_warning(str, ...)
#endif
#ifdef UnityLog
#define av_log_error(str, ...) Debug::LogError(str, ##__VA_ARGS__)
#define av_log_info(str, ...) Debug::Log(str, ##__VA_ARGS__)
#define av_log_pframe(str, ...)
#define av_log_warning(str, ...)
#endif // UnityLog

#ifdef NoneLog
#define av_log_error(str, ...)
#define av_log_info(str, ...)
#define av_log_pframe(str, ...)
#define av_log_warning(str, ...)
#endif // UnityLog
//#define av_log_pframe(str,...) Debug::Log(str, ##__VA_ARGS__)
#define MONOLAYOUT                                      \
    {                                                   \
        AV_CHANNEL_ORDER_NATIVE, (1), AV_CH_LAYOUT_MONO \
    }
#define STEREOLAYOUT                                      \
    {                                                     \
        AV_CHANNEL_ORDER_NATIVE, (2), AV_CH_LAYOUT_STEREO \
    }
void UnityLogCallbackFunc(void *, int, const char *, va_list);
void LogCallbackTotxt(void *, int, const char *, va_list);
void WindowsCallbackFunc(void *, int, const char *, va_list);
class Debug
{
public:
    static void (*LogFunPtr)(char *message, int size);
    static void (*LogErrorFunPtr)(char *message, int size);
    static void Log(const char *msg, ...);
    static void LogError(const char *msg, ...);
};