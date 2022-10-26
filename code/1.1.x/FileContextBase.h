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
    //����formatcontext��Ϣ
    FileContextBase();
    void ResetFormatContextByFileName(const char *fileName);
    //�������ݰ�
    void DecodePakcet();
    virtual void DealVideoPacket();
    virtual void DealAudioPacket();

    //��ȡ����Ϣ
    void FindStreamInformation();
    void FindStreamInformation(AVFormatContext *formatContext);
    std::pair<int, int> getAVStreamIndices() const;
    int getAudioStreamIndex() const;
    int getVideoStreamIndex() const;
    inline std::pair<int, int> getAVStreamIndices(AVFormatContext *formatContext) const;
    int getAudioStreamIndex(AVFormatContext *formatContext) const;
    int getVideoStreamIndex(AVFormatContext *formatContext) const;
    //��ӡ��Ƶ������Ϣ

public:
    FileContextBase(const char *srcFilePath);
    void DumpFormatContextInfo(const char *srcFilePath) const;
    void GetInformation();
    virtual ~FileContextBase();
};