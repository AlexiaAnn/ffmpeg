#pragma once
#include "IncludeFFmpeg.h"
#include "PlayerMisco.h"
#include <queue>
#include <mutex>
#include <condition_variable>
class PacketQueue
{
public:
	// AVFifoBuffer *pkt_list;
	std::queue<MyAVPacketList> pktQueue;
	int nb_packets;
	int size;
	int64_t duration;
	int abort_request;
	int serial;
	std::mutex queueMutex;
	std::condition_variable cond;
	int packetAllCount;

private:
	int packet_queue_put_private(AVPacket *pkt);

public:
	PacketQueue() : size(0), duration(0), abort_request(0), serial(1), nb_packets(0), packetAllCount(0){}
	int PacketQueueInit();
	int PacketQueuePut(AVPacket *pkt);
	int PacketQueueGet(AVPacket* pkt,int block, int* serial);
	bool HasEnoughPackets(AVStream *stream, int stream_id);
	void Flush();
	void PacketQueueStart();
	void packet_queue_abort();
	void destory();
	~PacketQueue();
};