#pragma once
#include "util.h"
#include <utility>
class FileContextBase
{
protected:
    AVFormatContext *inFmtCtx;
    AVPacket *dePacket;
    AVDictionary *format_opts;
    int deAudioPacketNumber;
    int deVideoPacketNumber;
    int ret;

private:
    void DecodePacket(AVFormatContext *formatContext, AVPacket *dePacket);
    virtual void DealVideoPacket(AVPacket *deVideoPacket);
    virtual void DealAudioPacket(AVPacket *deAudioPacket);

protected:
    //设置formatcontext信息
    FileContextBase();
    void ResetFormatContextByFileName(const char *fileName);
    //解析数据包
    void DecodePakcet();
    virtual void DealVideoPacket();
    virtual void DealAudioPacket();

    //获取流信息
    void FindStreamInformation();
    void FindStreamInformation(AVFormatContext *formatContext);
    std::pair<int, int> getAVStreamIndices() const;
    int getAudioStreamIndex() const;
    int getVideoStreamIndex() const;
    inline std::pair<int, int> getAVStreamIndices(AVFormatContext *formatContext) const;
    int getAudioStreamIndex(AVFormatContext *formatContext) const;
    int getVideoStreamIndex(AVFormatContext *formatContext) const;
    //打印视频整体信息

public:
    FileContextBase(const char *srcFilePath);
    void DumpFormatContextInfo(const char *srcFilePath) const;
    void GetInformation();
    virtual ~FileContextBase();
};