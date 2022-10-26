#pragma once
#include "IncludeFFmpeg.h"
#include "PacketQueue.h"
#include <condition_variable>
#include "FrameQueue.h"
class Decoder
{
public:
    AVPacket *pkt;
    PacketQueue *queue;
    AVCodecContext *avctx;
    int pkt_serial;
    int finished;
    int packet_pending;
    std::condition_variable* empty_queue_cond;
    int64_t start_pts;
    AVRational start_pts_tb;
    int64_t next_pts;
    AVRational next_pts_tb;
    std::thread* decoder_tid;
    int typeId;

public:
    Decoder() : empty_queue_cond(nullptr),pkt(nullptr), queue(nullptr), avctx(nullptr), pkt_serial(0), finished(0), packet_pending(0), start_pts(0), start_pts_tb(), next_pts(0), next_pts_tb() {}
    int DecoderInit(AVCodecContext* avctx, PacketQueue* queue, std::condition_variable* empty_queue_cond);
    int DecoderStart(std::thread* decoder_tid,int id);
    void DecoderAbort(FrameQueue* fq);
    void Destory();
    ~Decoder();
};