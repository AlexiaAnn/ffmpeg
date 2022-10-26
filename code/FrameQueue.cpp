#include "FrameQueue.h"

FrameQueue::FrameQueue(PacketQueue *pktq, int max_size, int keep_last)
{
}
int FrameQueue::FrameQueueInit(PacketQueue *pktq, int max_size, int keep_last)
{
    this->pktq = pktq;
    this->max_size = min(max_size, FRAME_QUEUE_SIZE);
    this->keep_last = !!keep_last;
    for (int i = 0; i < this->max_size; ++i)
    {
        if (!(queue[i].frame = av_frame_alloc()))
            return AVERROR(ENOMEM);
    }
    return 0;
}
int FrameQueue::FrameQueueNbRemaining() const
{
    return size - rindex_shown;
}

void FrameQueue::FrameQueuePush()
{
    if (++windex == max_size)
        windex = 0;
    mtx.lock();
    allcount++;
    size++;
    // av_log_info("push a frame,now count is:%d\n",allcount);
    cond.notify_all(); // todo all or one
    mtx.unlock();
}

Frame *FrameQueue::frame_queue_peek_writable()
{
    bool isWait = false;
    mtx.lock();
    while (true)
    {
        if (isWait)
            mtx.lock();
        if (!(size >= max_size &&
              !pktq->abort_request))
            break;
        isWait = true;
        std::adopt_lock_t t;
        std::unique_lock<std::mutex> lock(mtx, t);
        cond.wait_until(lock, std::chrono::system_clock::now() + std::chrono::milliseconds(10));
    }
    mtx.unlock();
    if (pktq->abort_request)
        return nullptr;
    return &queue[windex];
}

Frame *FrameQueue::frame_queue_peek_last()
{
    return &(queue[this->rindex]);
}

Frame *FrameQueue::frame_queue_peek()
{
    return &this->queue[(this->rindex + this->rindex_shown) % this->max_size];
}

Frame *FrameQueue::frame_queue_peek_next()
{
    return &this->queue[(this->rindex + this->rindex_shown + 1) % this->max_size];
}

Frame *FrameQueue::frame_queue_peek_readable()
{
    /* wait until we have a readable a new frame */
    bool isWait = false;
    mtx.lock();
    while (true)
    {
        if (isWait)
            mtx.lock();
        if (!(size - rindex_shown <= 0 &&
              !pktq->abort_request))
            break;
        isWait = true;
        std::adopt_lock_t t;
        std::unique_lock<std::mutex> lock(mtx, t);
        cond.wait_until(lock, std::chrono::system_clock::now() + std::chrono::milliseconds(10));
    }
    mtx.unlock();
    if (pktq->abort_request)
        return nullptr;
    return &queue[(rindex + rindex_shown) % max_size];
}

void FrameQueue::frame_queue_next()
{
    if (keep_last && !rindex_shown)
    {
        rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(&queue[rindex]);
    if (++rindex == max_size)
        rindex = 0;
    mtx.lock();
    size--;
    cond.notify_all(); // todo all or one
    mtx.unlock();
}

void FrameQueue::frame_queue_unref_item(Frame *vp)
{
    av_frame_unref(vp->frame);
    // avsubtitle_free(&vp->sub);//todo 暂时是不需要的
}

void FrameQueue::frame_queue_signal()
{
    this->mtx.lock();
    this->cond.notify_all(); // todo all or one
    this->mtx.unlock();
}

void FrameQueue::destory()
{
    int i;
    for (i = 0; i < max_size; i++)
    {
        Frame *vp = &queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
}

FrameQueue::~FrameQueue()
{
    // todo
}
