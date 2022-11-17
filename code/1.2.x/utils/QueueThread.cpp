#include "QueueThread.h"

QueueThread::QueueThread():mMaxSize(3)
{
}

void QueueThread::Push(IdFrame frame)
{
    std::unique_lock<std::mutex> locker(mMutex);
    while (isFull()) {
        av_log_info("avframe queue is full,please wait");
        mNotFull.wait(locker);
    }
    videoFrameQueue.push(frame);
    mNotEmpty.notify_one();
}

AVFrame* QueueThread::Pop()
{
    std::unique_lock <std::mutex> locker(mMutex);
    while (isEmpty()) {
        av_log_info("avframe queue is empty,please wait");
        mNotEmpty.wait(locker);
    }
    AVFrame* result = videoFrameQueue.top().second;
    videoFrameQueue.pop();
    mNotFull.notify_one();
    return result;
}
