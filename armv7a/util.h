#pragma once
#define __STDC_CONSTANT_MACROS
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
}
#include <iostream>
#include <unordered_map>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <memory>
// #define av_log_error(str, ...) av_log(nullptr, AV_LOG_ERROR, str, __VA_ARGS__)
// #define av_log_info(str, ...) av_log(nullptr, AV_LOG_INFO, str, __VA_ARGS__)
//#define av_log_error(str,...) logger->info(str,__VA_ARGS__)
//#define av_log_info(str,...) logger->info(str,__VA_ARGS__)
#define av_log_error(str, ...)
#define av_log_info(str, ...)
//通过文件名获取formatcontext
AVFormatContext *GetFormatContextByFileName(const char *inVideoFileName, std::shared_ptr<spdlog::logger> logger);
//获取解码器上下文
int Open_decodec_context(AVStream *stream, AVCodecContext **codecContext, AVFormatContext *fmtContext, AVMediaType type, std::shared_ptr<spdlog::logger> logger);
//获取编码器上下文
int Open_encodec_context(AVStream *stream, AVCodecID targetCodecID, AVCodecContext **codecContext, std::shared_ptr<spdlog::logger> logger);

AVCodecContext *AllocEncodecContext(AVCodecID codecID);
int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);
//加载formatcontext中的流信息，比如视频流和音频流
bool Find_stream_info(AVFormatContext *formatContext, std::shared_ptr<spdlog::logger> logger);
//获取视频dump信息
void GetVideoInformation(const char *filePath);
AVFrame *CreateAvFrame();
AVPacket *CreateAvPacket();