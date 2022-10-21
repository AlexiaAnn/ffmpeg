#include "FileContextBase.h"

FileContextBase::FileContextBase() : inFmtCtx(nullptr), dePacket(nullptr),
                                     deAudioPacketNumber(0), deVideoPacketNumber(0)
{
}
void FileContextBase::ResetFormatContextByFileName(const char *fileName)
{

    if (inFmtCtx == nullptr || strcmp(inFmtCtx->url, fileName) != 0) {
        av_log_info("formatcontext is null or url is wrong,resetting the formatcontext\n");
        avformat_free_context(inFmtCtx);
        inFmtCtx = GetFormatContextByFileName(fileName);
        FindStreamInformation();
        av_log_info("resetting the formatcontext over\n");
    }
}
void FileContextBase::DecodePakcet()
{
    std::pair<int, int> streamIndices = getAVStreamIndices();
    while (av_read_frame(inFmtCtx, dePacket) >= 0)
    {
        if (dePacket->stream_index == streamIndices.first)
        {
            DealAudioPacket();
        }
        else if (dePacket->stream_index == streamIndices.second)
        {
            DealVideoPacket();
        }
    }
    av_log_info("decode all packet,over\n");
}
void FileContextBase::DealAudioPacket()
{
}
void FileContextBase::DealVideoPacket()
{
}
void FileContextBase::DecodePacket(AVFormatContext* formatContext, AVPacket* packet)
{
    std::pair<int, int> streamIndices = getAVStreamIndices();
    while (av_read_frame(formatContext, packet) >= 0)
    {
        if (packet->stream_index == streamIndices.first)
        {
            DealAudioPacket(packet);
        }
        else if (packet->stream_index == streamIndices.second)
        {
            DealVideoPacket(packet);
        }
    }
    av_log_info("decode all packet,over\n");
}
void FileContextBase::DealVideoPacket(AVPacket* deVideoPacket)
{
    deVideoPacketNumber++;
}
void FileContextBase::DealAudioPacket(AVPacket* deAudioPacket)
{
    deAudioPacketNumber++;
}

void FileContextBase::FindStreamInformation()
{
    FindStreamInformation(inFmtCtx);
}
void FileContextBase::FindStreamInformation(AVFormatContext* formatContext)
{
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0)
    {
        av_log_info("could not find stream information\n");
    }
}
std::pair<int, int> FileContextBase::getAVStreamIndices() const
{
    return getAVStreamIndices(inFmtCtx);
}
int FileContextBase::getAudioStreamIndex() const
{
    return getAudioStreamIndex(inFmtCtx);
}
int FileContextBase::getVideoStreamIndex() const
{
    return getVideoStreamIndex(inFmtCtx);
}
std::pair<int, int> FileContextBase::getAVStreamIndices(AVFormatContext* formatContext) const
{
    int audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    int videoStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    return { audioStreamIndex, videoStreamIndex };
}
int FileContextBase::getAudioStreamIndex(AVFormatContext* formatContext) const
{
    return av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
}
int FileContextBase::getVideoStreamIndex(AVFormatContext* formatContext) const
{
    return av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
}
void FileContextBase::DumpFormatContextInfo(const char *srcFilePath) const
{
    if (inFmtCtx == nullptr)
    {
        av_log_error("inFmtCtx is nullptr so far, it is not be initialized,so can not dump the formatcontext information\n");
        return;
    }
    av_dump_format(inFmtCtx, 0, srcFilePath, 0);
    av_log_info("video packet:%d,audio packet:%d",deVideoPacketNumber,deAudioPacketNumber);
}
void FileContextBase::GetInformation()
{
    DecodePacket(inFmtCtx,dePacket);
    DumpFormatContextInfo(inFmtCtx->url);
}
FileContextBase::FileContextBase(const char *srcFilePath) : inFmtCtx(GetFormatContextByFileName(srcFilePath)), dePacket(AllocAVPacket()),
                                                            deAudioPacketNumber(0), deVideoPacketNumber(0)
{
    FindStreamInformation();
    av_log_info("file context base init over\n");
    if (ret >= 0) {
        av_log_info("file context base init success\n");
    }
    else {
        av_log_error("file context base init failed\n");
    }
}
FileContextBase::~FileContextBase()
{
    av_packet_free(&dePacket);
    avformat_free_context(inFmtCtx);
}
