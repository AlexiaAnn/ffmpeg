#include "PacketQueue.h"

int PacketQueue::PacketQueueInit()
{
	abort_request = 1;
	return 0;
}
int PacketQueue::packet_queue_put_private(AVPacket *pkt)
{
	MyAVPacketList pkt1;

	if (abort_request)
		return -1;

	pkt1.pkt = pkt;
	pkt1.serial = serial;

	pktQueue.push(pkt1);
	nb_packets++;
	size += pkt1.pkt->size + sizeof(pkt1);
	duration += pkt1.pkt->duration;
	packetAllCount++;
	// av_log_info("packet queue put a packet,stream_index:%d,packet_index:%d\n", pkt1.pkt->stream_index,packetAllCount);
	/* XXX: should duplicate packet data in DV case */
	cond.notify_all(); // todo 是用all还是one
	return 0;
}
int PacketQueue::PacketQueuePut(AVPacket *pkt)
{
	AVPacket *pkt1;
	int ret;

	pkt1 = av_packet_alloc();
	if (!pkt1)
	{
		av_packet_unref(pkt);
		return -1;
	}
	av_packet_move_ref(pkt1, pkt);

	queueMutex.lock();
	ret = packet_queue_put_private(pkt1);
	queueMutex.unlock();

	if (ret < 0)
		av_packet_free(&pkt1);

	return ret;
}
int PacketQueue::PacketQueueGet(AVPacket *pkt, int block, int *serial)
{
	MyAVPacketList pkt1;
	int ret = 0;
	int isWait = false;
	queueMutex.lock();

	while (true)
	{
		if (isWait)
			queueMutex.lock();
		if (abort_request)
		{
			ret = -1;
			break;
		}

		if (!pktQueue.empty())
		{
			pkt1 = pktQueue.front();
			pktQueue.pop();
			nb_packets--;
			size -= pkt1.pkt->size + sizeof(pkt1);
			duration -= pkt1.pkt->duration;
			av_packet_move_ref(pkt, pkt1.pkt);
			if (serial)
				*serial = pkt1.serial;
			av_packet_free(&pkt1.pkt);
			ret = 1;
			break;
		}
		else if (!block)
		{
			ret = 0;
			break;
		}
		else
		{
			isWait = true;
			std::adopt_lock_t t;
			std::unique_lock<std::mutex> lock(queueMutex, t);
			cond.wait(lock);
		}
	}
	queueMutex.unlock();
	return ret;
}
bool PacketQueue::HasEnoughPackets(AVStream *stream, int stream_id)
{
	if (stream == nullptr)
		return true;
	bool b1 = stream_id < 0;
	bool b2 = (stream->disposition & AV_DISPOSITION_ATTACHED_PIC);
	bool b3 = nb_packets > MIN_FRAMES && (!duration || av_q2d(stream->time_base) * duration > 1.0);
	return b1 || abort_request || b2 || b3;
}
void PacketQueue::Flush()
{
	queueMutex.lock();
	while (!pktQueue.empty())
	{
		av_packet_free(&pktQueue.front().pkt);
		pktQueue.pop();
	}
	nb_packets = 0;
	size = 0;
	duration = 0;
	serial++;
	queueMutex.unlock();
}

void PacketQueue::PacketQueueStart()
{
	queueMutex.lock();
	abort_request = 0;
	serial++;
	queueMutex.unlock();
}

void PacketQueue::packet_queue_abort()
{
	this->queueMutex.lock();
	this->abort_request = 1;
	this->cond.notify_all(); // todo all or one
	this->queueMutex.unlock();
}

void PacketQueue::destory()
{
	this->Flush();
}

PacketQueue::~PacketQueue()
{
	this->Flush();
}
