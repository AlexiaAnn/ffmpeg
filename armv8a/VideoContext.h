#pragma once
#include "ContextBase.h"
#include <queue>
#include <mutex>
class VideoContext : public ContextBase
{
protected:
    SwsContext* swsCtx;
    AVCodecContext* deVideoCodecCtx;
    AVCodecContext* enVideoCodecCtx;

    AVFormatContext* outFmtCtx;
    AVCodecID enCodecId;
    AVFrame* deVideoFrame;
    AVFrame* enVideoFrame;
    AVPacket* enVideoPacket;
    int pts;

    // multithread
    std::mutex frameMutex;
    std::mutex frameNumberOfQueueMutex;
    std::queue<std::pair<void*, int>> frameQueue;

    int frameNumber;

public:
    bool isEnd;
    FILE* dstFilePtr;
    FILE* srcFilePtr;

protected:
    AVCodecContext* OpenVideoEncodecContext(AVCodecID codecId, int fps, int width, int height);
    SwsContext* AllocSwsContext(AVCodecContext* deCodecCtx, AVCodecContext* enCodecCtx);
    SwsContext* AllocSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight, AVCodecContext* enCodecCtx);
    AVFrame* CreateVideoAVFrame(AVCodecContext* codecContxt);
    void RescaleDeVideoFrame();
    void RescaleDevideoFrame(uint8_t** data, int* linesize);
    void EncodeVideoFrame();
    void EncodeVideoFrame(AVFrame* frame);
    void DealVideoPacket(AVPacket* packet) override;

public:
    VideoContext() = delete;
    VideoContext(AVCodecID codecId, int fps, int width, int height);
    VideoContext(AVPixelFormat dePixFormat, AVCodecID codecId, int fps, int width, int height);
    VideoContext(AVPixelFormat dePixFormat, AVCodecID codecId, int fps, int deWidth, int deHeight, int enWidth, int enHeight);
    void SetOutFormatContextByFileName(const char* dstFilepath);
    void WriteFramePrepare(const char* dstFilePath);
    void ConsumeFrameFromQueue();
    void WriteFrameTailer();
    void ConsumeFrameFromtQueueToRGBFile(const char* dstFilePath);
    void AddFrameToQueue(void* dataPtr, int length);
    //void WriteFrameStart(const char* dstFilePath);
    void WriteFrame(void* data, int length);
    void WriteFrameToYUVFile(void* data, int length);
    void WriteFrameFromRGBFile(void* data, int length);
    int GetQueueSize();
    bool ExtractVideoToFile(const char* srcFilePath, const char* dstFilePath);
    bool ExtractRGBToFile(const char* srcFilePath, const char* dstFilePath);
    virtual ~VideoContext();
};