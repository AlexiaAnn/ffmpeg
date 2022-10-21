#include "InFormatContext.h"
InFormatContext::InFormatContext(const char* srcFilePath) :ret(0) {
    inFmtContext = GetFormatContextByFileName(srcFilePath);
    if (inFmtContext == nullptr) goto end;
    avformat_find_stream_info(inFmtContext,nullptr);
    dePacket = AllocAVPacket();
    if (dePacket == nullptr) goto end;
    return;
end:
    ret = -1;
    av_log_error("formatcontext initial failed\n");
    return;
}
int InFormatContext::GetResult() const {
    return ret;
}
InFormatContext::~InFormatContext() {
    avformat_free_context(inFmtContext);
    av_packet_free(&dePacket);
}

void InFormatContext::DumpFileInfo() const {
    av_dump_format(inFmtContext, 0, inFmtContext->url, 0);
}


//stream
AVStream* InFormatContext::GetVideoStream() const {
    int index = av_find_best_stream(inFmtContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (index < 0) return nullptr;
    return inFmtContext->streams[index];
}
AVStream* InFormatContext::GetAudioStream() const {
    int index = av_find_best_stream(inFmtContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (index < 0) return nullptr;
    return inFmtContext->streams[index];
}
AVStream* InFormatContext::GetVideoStreamByWhile() const {
    for (int i = 0; i < inFmtContext->nb_streams; ++i) {
        AVStream* stream = inFmtContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) return stream;
    }
    return nullptr;
}
AVStream* InFormatContext::GetAudioStreamByWhile() const {
    for (int i = 0; i < inFmtContext->nb_streams; ++i) {
        AVStream* stream = inFmtContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) return stream;
    }
    return nullptr;
}
AVPacket* InFormatContext::GetNextPacket() {
    if (av_read_frame(inFmtContext, dePacket) >= 0)
        return dePacket;
    av_log_info("depacket over");
    return nullptr;
}

//informatcontext
AVFormatContext* InFormatContext::GetInFormatContext() const {
    return inFmtContext;
}
