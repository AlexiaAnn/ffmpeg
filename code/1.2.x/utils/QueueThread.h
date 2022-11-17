#pragma once
#include "IncludeFFmpeg.h"
#include <queue>
#include <mutex>
#include <condition_variable>
class QueueThread
{
private:
	using IdFrame = std::pair<int, AVFrame*>;
	std::priority_queue<IdFrame, std::vector<IdFrame>, std::greater<IdFrame>> videoFrameQueue;

	std::mutex mMutex;
	std::condition_variable mNotEmpty;
	std::condition_variable mNotFull;
	int mMaxSize = 3;

private:
	bool isEmpty() const {
		return videoFrameQueue.empty();
	};
	bool isFull() const {
		return videoFrameQueue.size() == mMaxSize;
	};
public:
	QueueThread();
	QueueThread(int maxSize) {
		this->mMaxSize = maxSize;
	};
	void Push(IdFrame frame);
	AVFrame* Pop();
	~QueueThread() {};
};

