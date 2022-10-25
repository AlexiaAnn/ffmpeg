#include "DeCodecContext.h"

DeCodecContext::DeCodecContext(AVStream* stream) :ret(0) {
    if (stream == nullptr) {
        ret = -1;
        return;
    }
    codecCont = OpenDecodecContextByStream(stream);
    if (codecCont == nullptr) {
        av_log_error("open decodec context error,DeCodecContext initial failed\n");
        ret = -1;
        return;
    }
    streamIndex = stream->index;
    deFrame = AllocAVFrame();
    if (deFrame == nullptr) {
        ret = -1;
        return;
    }
}
AVFrame* DeCodecContext::GetReceiveFrame() { 
    av_frame_unref(deFrame);//error prone
    ret = avcodec_receive_frame(codecCont, deFrame);
    if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
        av_log_warning("there are no enough frame data in buffer\n");
        return nullptr;
    }
    if (ret < 0) {
        av_log_error("error during decoding\n");
        return nullptr;
    }
    
    return deFrame;
}
AVFrame* DeCodecContext::GetNextFrame(InFormatContext& infmtCont)
{
    //首先从解码器中解码已经缓存的packet数据
    AVFrame* frame = nullptr;
    while (frame == nullptr) {
        frame = GetReceiveFrame();
        if (frame == nullptr) {
            AVPacket* packet;
            while (true) {
                packet = infmtCont.GetNextPacket();
                if (packet == nullptr) return nullptr;
                else if (packet->stream_index != streamIndex) continue;
                else {
                    SendPacket(packet);
                    break;
                }
            }

        }
    }
    return frame;
}
bool DeCodecContext::SendPacket(AVPacket* packet)
{
    if (packet->stream_index != streamIndex) {
        av_log_error("packet stream index != target stream index");
        return false;
    }
    ret = avcodec_send_packet(codecCont,packet);
    if (ret < 0) {
        av_log_error("Error submitting a packet for deconding\n");
        return false;
    }
    return true;
}
AVCodecContext* DeCodecContext::GetCodecContext() const
{
    return codecCont;
}
DeCodecContext::~DeCodecContext() {
    avcodec_free_context(&codecCont);
    av_frame_free(&deFrame);
}

int DeCodecContext::GetResult() const
{
    return ret;
}
