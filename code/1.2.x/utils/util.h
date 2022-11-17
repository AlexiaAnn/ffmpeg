#pragma once
#include "IncludeFFmpeg.h"
AVPacket* AllocAVPacket();
AVFrame* AllocAVFrame();
AVFormatContext* GetFormatContextByFileName(const char* fileName);
AVFormatContext* AllocOutFormatContext(const char* dstFileName);
int AddNewStreamToFormat(AVFormatContext* outFormatContext, AVCodecContext* codecContext);
AVCodecContext* AllocEncodecContext(AVCodecID codecId);
AVCodecContext* AllocEncodecContext(const char* codecName);
AVCodecContext* OpenDecodecContextByStream(AVStream* stream);
AVCodecContext* OpenDecodecContextByStream(const char* codecName,AVStream* stream);
const char* GetSampleFormatString(AVSampleFormat format);
AVFilterInOut* AllocAVFilterInOut();
AVFilterGraph* AllocAVFilterGraph();
AVFrame* CreateVideoFrame(AVPixelFormat format, int width, int height);
void FlipImage(unsigned char* src,int width,int height);
void FlipImage(uint8_t* data,int width,int height,int channel);
AVPixelFormat GetPixFormatByCodecId(AVCodecID codecId);