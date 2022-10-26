#pragma once
#include "Frame.h"
#include <mutex>
#include "PacketQueue.h"
#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))
class FrameQueue
{
public:
    Frame queue[FRAME_QUEUE_SIZE];
    // std::queue<Frame> frameQueue;
    int rindex;
    int windex;
    int size;
    int max_size;
    int allcount;
    int keep_last;
    int rindex_shown;
    std::mutex mtx;
    std::condition_variable cond;
    PacketQueue *pktq;

public:
    FrameQueue() : rindex(0), windex(0), size(0), max_size(0), keep_last(0), rindex_shown(0),allcount(0), pktq(nullptr) {}
    FrameQueue(PacketQueue *pktq, int max_size, int keep_last);
    int FrameQueueInit(PacketQueue *pktq, int max_size, int keep_last);
    int FrameQueueNbRemaining() const;
    void FrameQueuePush();
    Frame* frame_queue_peek_writable();
    Frame* frame_queue_peek_last();
    Frame* frame_queue_peek();
    Frame* frame_queue_peek_next();
    Frame* frame_queue_peek_readable();
    void frame_queue_next();
    void frame_queue_unref_item(Frame* vp);
    void frame_queue_signal();
    void destory();
    ~FrameQueue();
};