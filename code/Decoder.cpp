#include "Decoder.h"

int Decoder::DecoderInit(AVCodecContext* avctx, PacketQueue* queue, std::condition_variable* empty_queue_cond)
{
    pkt = av_packet_alloc();
    if (!pkt)
        return AVERROR(ENOMEM);
    this->avctx = avctx;
    this->queue = queue;
    this->empty_queue_cond = empty_queue_cond;
    this->start_pts = AV_NOPTS_VALUE;
    this->pkt_serial = -1;
    return 0;
}

int Decoder::DecoderStart(std::thread* decoder_tid,int id)
{
    this->queue->PacketQueueStart();
    this->decoder_tid = decoder_tid;
    this->typeId = id;
    return 0;
}

void Decoder::DecoderAbort(FrameQueue* fq)
{
    this->queue->packet_queue_abort();
    fq->frame_queue_signal();
    int result = typeId;
    //decoder_tid->join();
    //decoder_tid = nullptr;
    this->queue->Flush();
}

void Decoder::Destory()
{
    av_packet_free(&pkt);
}

Decoder::~Decoder()
{
}
