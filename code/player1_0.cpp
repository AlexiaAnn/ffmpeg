//废弃代码
int NewPlayer::ReceiveVideoFrame(AVFrame *frame)
{
    int gotPicture;
    if ((gotPicture = DecoderFrame(deVideoCodecCtx, frame, videoPacketQueue, videoPacketQueueMutex, videoPacketQueueBytesSize, videoPacketsDuration)) < 0)
    {
        return -1;
    }
    if (gotPicture)
    {
        double dpts = NAN;
        if (frame->pts != AV_NOPTS_VALUE)
            dpts = av_q2d(videoStream->time_base) * frame->pts;
        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(inFmtCtx, videoStream, frame);
        //暂时先只考虑视频同步音频,需要对慢的帧进行丢弃，以追赶音频帧 todo
        if (frame->pts != AV_NOPTS_VALUE)
        {
            double diff = dpts - audclk.GetClock();
            //暂时未加的判断: is->viddec.pkt_serial == is->vidclk.serial is->videoq.nb_packets
            if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD && diff - frame_last_filter_delay < 0)
            {
                ++frame_drops_early;
                av_frame_unref(frame);
                gotPicture = 0;
            }
        }
    }
    return gotPicture;
    //首先从解码器中解码已经缓存的packet数据
    // while (eof == false) {
    //    if (deVideoFrame == nullptr) deVideoFrame = AllocAVFrame();
    //    if (videoFrameQueue.size() > 10) {
    //        av_frame_free(&videoFrameQueue.front());
    //        videoFrameQueue.pop();
    //    }
    //    ret = avcodec_receive_frame(deVideoCodecCtx, deVideoFrame);
    //    if (ret >= 0) {
    //        videoFrameCount += 1;
    //        videoFrameQueue.push(deVideoFrame);
    //        av_log_error("get a video frame\n");
    //        continue;
    //    }
    //    packetsQueueMutex.lock();
    //    if (packets.empty() || packets.front()->stream_index != videoStream->index) {
    //        packetsQueueMutex.unlock();
    //        continue;
    //    }
    //    AVPacket* packet = packets.front();
    //    packets.pop();
    //    packetsQueueMutex.unlock();
    //    //还有packet，从中获取新的frame
    //    ret = avcodec_send_packet(deVideoCodecCtx, packet);
    //    if (ret < 0)
    //    {
    //        av_log_error("Error submitting a packet for deconding\n");
    //        continue;
    //    }
    //    if (avcodec_receive_frame(deVideoCodecCtx, deVideoFrame) >= 0) {
    //        videoFrameCount += 1;
    //        videoFrameQueue.push(deVideoFrame);
    //        av_log_error("get a video frame\n");
    //    }
    //}
    // av_log_info("receive video frame over\n");
}
void NewPlayer::ReceiveAudioFrame()
{
    //首先从解码器中解码已经缓存的packet数据
    // while (eof==false) {
    //    if (deAudioFrame == nullptr) deAudioFrame == AllocAVFrame();
    //    ret = avcodec_receive_frame(deAudioCodecCtx, deAudioFrame);
    //    if (ret >= 0) {
    //        audioFrameCount += 1;
    //        audioFrameQueue.push(deAudioFrame);
    //        av_log_error("get a audio frame\n");
    //        continue;
    //    }
    //    packetsQueueMutex.lock();
    //    if (packets.empty() || packets.front()->stream_index != audioStream->index) {
    //        packetsQueueMutex.unlock();
    //        continue;
    //    }
    //    AVPacket* packet = packets.front();
    //    packets.pop();
    //    packetsQueueMutex.unlock();
    //    //还有packet，从中获取新的frame
    //    ret = avcodec_send_packet(deAudioCodecCtx, packet);
    //    if (ret < 0)
    //    {
    //        av_log_error("Error submitting a packet for deconding\n");
    //        continue;
    //    }
    //    if (avcodec_receive_frame(deAudioCodecCtx, deAudioFrame) >= 0) {
    //        audioFrameCount += 1;
    //        audioFrameQueue.push(deAudioFrame);
    //        av_log_error("get a audio frame\n");
    //    }
    //}
    // av_log_info("receive audio frame over\n");
}
void NewPlayer::VideoThread()
{
    if (deVideoFrame == nullptr)
        deVideoFrame = AllocAVFrame();
    AVRational frameRate = av_guess_frame_rate(inFmtCtx, videoStream, nullptr);
    double duration;
    double pts;
    while (true)
    {
        ret = ReceiveVideoFrame(deVideoFrame);
        if (ret < 0)
            break;
        if (!ret)
            continue;
        duration = (frameRate.num && frameRate.den ? av_q2d({frameRate.den, frameRate.num}) : 0);
        pts = (deVideoFrame->pts == AV_NOPTS_VALUE) ? NAN : deVideoFrame->pts * av_q2d(videoStream->time_base);
        //将frame放入queue todo
        ret = PutVideoFrameToQueue(deVideoFrame, pts, duration);
        av_frame_unref(deVideoFrame);
        if (ret < 0)
            break;
    }
}
void NewPlayer::AudioThread()
{
}

int NewPlayer::DecoderFrame(AVCodecContext *codecCtx, AVFrame *frame, std::queue<AVPacket *> &avPacketQueue, std::mutex &mtx,
                            int64_t &byteSize, int64_t &duration)
{
    int result = AVERROR(EAGAIN);
    AVPacket *packet;
    while (true)
    {
        // if (d->queue->serial == d->pkt_serial) 暂时不考虑 todo
        //优先读取解码器缓存中的数据，将其解析为帧
        do
        {
            if (abort_request)
                return -1;
            switch (codecCtx->codec_type)
            {
            case AVMEDIA_TYPE_VIDEO:
                result = avcodec_receive_frame(codecCtx, frame);
                if (ret >= 0)
                {
                    frame->pts = frame->best_effort_timestamp;
                }
                break;
            case AVMEDIA_TYPE_AUDIO:
                result = avcodec_receive_frame(codecCtx, frame);
                if (ret >= 0)
                {
                    frame->pts = frame->best_effort_timestamp;
                    AVRational tb = {1, frame->sample_rate};
                    if (frame->pts != AV_NOPTS_VALUE)
                        frame->pts = av_rescale_q(frame->pts, codecCtx->pkt_timebase, tb);
                    else if (audioNextPts != AV_NOPTS_VALUE)
                        frame->pts = av_rescale_q(audioNextPts, audioNextPtsTb, tb);
                    if (frame->pts != AV_NOPTS_VALUE)
                    {
                        audioNextPts = frame->pts + frame->nb_samples;
                        audioNextPtsTb = tb;
                    }
                }
                break;
            }
            if (ret == AVERROR_EOF)
            {
                avcodec_flush_buffers(codecCtx);
                return 0;
            }
            if (ret >= 0)
                return 1;
        } while (result != AVERROR(EAGAIN));
        //缓存中没有数据，消费packet对解码器进行填充
        while (true)
        {
            if (avPacketQueue.empty())
            {
                readPacketCV.notify_one();
            }
            else
            {
                mtx.lock();
                packet = avPacketQueue.front();
                avPacketQueue.pop();
                byteSize -= packet->size + sizeof(packet);
                duration -= packet->duration;
                mtx.unlock();
                break;
            }
        }
        if (avcodec_send_packet(codecCtx, packet) == AVERROR(EAGAIN))
        {
        }
        else
        {
            av_packet_unref(packet);
            av_packet_free(&packet); // todo
        }
    }
}
int NewPlayer::PutVideoFrameToQueue(AVFrame *srcFrame, double pts, double duration)
{
    return 0;
}
int NewPlayer::PutPacketToQueue(std::mutex &mtx, std::queue<AVPacket *> &packetQueue, AVPacket *packet, int64_t &queueByteSize, int64_t &queueDuration)
{
    mtx.lock();
    //放入音频队列
    if (abort_request)
    {
        av_packet_free(&packet);
        return -1;
    }
    queueByteSize += packet->size + sizeof(packet);
    queueDuration += packet->duration;
    packetQueue.push(packet);
    mtx.unlock();
    return 0;
}
void NewPlayer::ReadPacket()
{
    InitReadPacketThreadData();
    while (true)
    {
        if (abort_request)
            break;
        if (paused)
            continue;
        if (audioPacketQueueBytesSize + videoPacketQueueBytesSize > MAX_QUEUE_SIZE)
        {
            // packet数据过多，挂起线程等待唤醒
            std::unique_lock<std::mutex> lock(readPacketMutex);
            av_log_info("线程被挂起，等待唤醒或超时唤醒\n");
            readPacketCV.wait_until(lock, std::chrono::system_clock::now() + std::chrono::milliseconds(10)); //挂起10ms，超时自动唤醒
            continue;
        }
        ret = av_read_frame(inFmtCtx, dePacket);
        if (ret == AVERROR_EOF || avio_feof(inFmtCtx->pb) && !eof)
        {
            //到达文件末尾，放入nullptr冲刷缓冲区
            audioPacketQueue.push(nullptr);
            videoPacketQueue.push(nullptr);
            break; //暂时决定结束线程 todo
        }
        else
            eof = false;
        //将packet按类型放入对应的队列
        AVPacket *tempPacket = AllocAVPacket();
        av_packet_move_ref(tempPacket, dePacket);
        if (tempPacket->stream_index == audioStream->index)
        {
            PutPacketToQueue(audioPacketQueueMutex, audioPacketQueue, tempPacket, audioPacketQueueBytesSize, audioPacketsDuration);
        }
        else if (tempPacket->stream_index == videoStream->index)
        {
            PutPacketToQueue(videoPacketQueueMutex, videoPacketQueue, tempPacket, videoPacketQueueBytesSize, videoPacketsDuration);
        }
        else
        {
            av_packet_unref(dePacket);
        }
        av_log_info("put a packet,audio queue duration:%d,video queue duration:%d\n", audioPacketsDuration, videoPacketsDuration);
    }
    av_log_info("de packet over,audio and video byte size:%d\n", audioPacketQueueBytesSize + videoPacketQueueBytesSize);
}
void NewPlayer::InitReadPacketThreadData()
{
    audioPacketQueueBytesSize = videoPacketQueueBytesSize = 0;
    ready = processed = false;
    audioPacketsDuration = videoPacketsDuration = packetQueueDuration = 0;
    if (dePacket == nullptr)
        dePacket = AllocAVPacket();
}
int NewPlayer::audio_open(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params)
{
    return 0;
}