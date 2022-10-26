#include "AVPlayer.h"
int AVPlayer::PtsToFrameIndex(int64_t pts)
{
    return av_rescale_q(pts, videoStream->time_base, {1, videoStream->r_frame_rate.num});
}
int64_t AVPlayer::FrameIndexToPts(int frameIndex)
{
    return av_rescale_q(frameIndex, {1, videoStream->r_frame_rate.num}, videoStream->time_base);
}
void AVPlayer::DealVideoPacket()
{
    packetsQueueMutex.lock();
    packets.push(dePacket);
    packetQueueDuration += dePacket->duration;
    packetsQueueMutex.unlock();
    av_packet_free(&dePacket);
    dePacket = AllocAVPacket();
    // if depacket = nullptr todo
}
void AVPlayer::DealAudioPacket()
{
    packetsQueueMutex.lock();
    packets.push(dePacket);
    packetsQueueMutex.unlock();
    av_packet_free(&dePacket);
    dePacket = AllocAVPacket();
    av_packet_free(&dePacket);
}
void AVPlayer::DealDeAudioFrame()
{
    int basepts = av_rescale_q(deAudioFrame->pts, audioStream->time_base, {1, AV_TIME_BASE});
    av_log_info("audio frame basepts:%d,pts:%ld ", basepts, deAudioFrame->pts);
}
void AVPlayer::DealDeVideoFrame()
{
    int basepts = av_rescale_q(deVideoFrame->pts, videoStream->time_base, {1, AV_TIME_BASE});
    av_log_info("\nvideo frame basepts:%d,pts:%ld ", basepts, deVideoFrame->pts);
}
AVPlayer::AVPlayer() : FileContextBase(), AVFileContext(), videoStream(nullptr), ret(0), curFrameIndex(-1)
{
}
AVPlayer::AVPlayer(const char *srcFilePath) : FileContextBase(srcFilePath),
                                              AVFileContext(AV_CODEC_ID_AAC, AV_PIX_FMT_YUV420P, AV_CODEC_ID_GIF,
                                                            inFmtCtx->streams[getVideoStreamIndex()]->codecpar->width,
                                                            inFmtCtx->streams[getVideoStreamIndex()]->codecpar->height,
                                                            inFmtCtx->streams[getVideoStreamIndex()]->r_frame_rate.num,
                                                            inFmtCtx->streams[getVideoStreamIndex()]->codecpar->width,
                                                            inFmtCtx->streams[getVideoStreamIndex()]->codecpar->height),
                                              ret(0), curFrameIndex(-1), isVideo(true), packetQueueDuration(0),
                                              audioPacketsDuration(0), videoPacketsDuration(0)
{
    enAudioFrame = AllocAVFrame();
    videoStream = inFmtCtx->streams[getVideoStreamIndex()];
    audioStream = inFmtCtx->streams[getAudioStreamIndex()];
    deVideoCodecCtx = OpenDecodecContextByStream(videoStream);
    deVideoFrame = AllocAVFrame();
    deVideoFrame->pts = -1;
    AudioFileContext::VariableCheck(srcFilePath);
    VideoFileContext::VariableCheck(srcFilePath);
    videoFmtCtx = inFmtCtx;
    audioFmtCtx = GetFormatContextByFileName(srcFilePath);
    deVideoPacket = dePacket;
    deAudioPacket = AllocAVPacket();
    audioLastPts = 0;
    audioLastTime = av_rescale_q(av_gettime(), {1, AV_TIME_BASE}, audioStream->time_base);
    audioNowPts = 0;
    isVideoEnd = false;
    isAudioEnd = false;

    GetNextVideoFrame();
    GetNextAudioFrame();
    swsCtx = InitSwsContext(deVideoCodecCtx, enVideoCodecCtx);
    swrCtx = InitSwrContext(deAudioCodecCtx, enAudioCodecCtx);
}

const AVFrame *AVPlayer::SeekFrameByFrameIndex(int frameIndex)
{
    if (frameIndex > videoStream->nb_frames)
        return nullptr;
    int64_t ts = FrameIndexToPts(frameIndex);
    av_log_info("target frame number:%d,its ts:%ld\n", frameIndex, ts);
    av_seek_frame(inFmtCtx, videoStream->index, ts, AVSEEK_FLAG_BACKWARD);
    dePacket->stream_index = videoStream->index + 1;
    while (dePacket->stream_index != videoStream->index)
    {
        ret = av_read_frame(inFmtCtx, dePacket);
    }
    curFrameIndex = PtsToFrameIndex(dePacket->pts);
    av_log_info("key frame number:%d\n", curFrameIndex);
    while (curFrameIndex != frameIndex)
    {
        if (dePacket->stream_index != videoStream->index)
        {
            ret = av_read_frame(inFmtCtx, dePacket);
            continue;
        }
        ret = avcodec_send_packet(deVideoCodecCtx, dePacket);
        if (ret < 0)
        {
            av_log_error("Error submitting a packet for deconding\n");
            return nullptr;
        }
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(deVideoCodecCtx, deVideoFrame);
            if (ret < 0)
            {
                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                    break;
                av_log_error("Error during decoding");
                break;
            }
            curFrameIndex = av_rescale_q(deVideoFrame->pts, videoStream->time_base, {1, videoStream->r_frame_rate.num});
            av_log_info("current frame number:%d,pts:%ld\n", curFrameIndex, deVideoFrame->pts);
            if (curFrameIndex == frameIndex)
                break;
            av_frame_unref(deVideoFrame);
            if (ret < 0)
                break;
        }
        ret = av_read_frame(inFmtCtx, dePacket);
    }
    curFrameIndex = PtsToFrameIndex(deVideoFrame->pts);
    av_log_info("target frame number:%d,result number:%d,it is %s\n", frameIndex, curFrameIndex, frameIndex == curFrameIndex ? "ok" : "not ok");
    RescaleDevideoFrame();
    return enVideoFrame;
}

const AVFrame *AVPlayer::GetFrameAtTargetPercent(float percent)
{
    int targetFrameIndex = int(percent * videoStream->nb_frames);

    // targetFrameIndex == now enframe targetFrameIndex ֱ�ӷ���
    if (targetFrameIndex == curFrameIndex)
        return enVideoFrame;
    else if (targetFrameIndex < curFrameIndex)
        return SeekFrameByFrameIndex(targetFrameIndex);
    // targetFrameIndex > now enframe index ˳���ȡ
    else
    {
        //���Ŀ��֡����������Ƶ��֡�������ؿ�
        if (targetFrameIndex > videoStream->nb_frames)
            return nullptr;
        //�����ǰ֡��Ŀ��֡������ֱ��seekЧ�ʱȽϸ�
        if (targetFrameIndex - curFrameIndex >= videoStream->r_frame_rate.num)
            return SeekFrameByFrameIndex(targetFrameIndex);
        //���������Ľ������еĻ���֡
        while (avcodec_receive_frame(deVideoCodecCtx, deVideoFrame) >= 0)
        {
            curFrameIndex = PtsToFrameIndex(deVideoFrame->pts);
            if (curFrameIndex < targetFrameIndex)
                continue;
            else if (curFrameIndex == targetFrameIndex)
            {
                RescaleDevideoFrame();
                return enVideoFrame;
            }
        }
        //�����껹��û�е�Ŀ��֡
        while (ret >= 0 && curFrameIndex < targetFrameIndex)
        {
            //����ֱ���ҵ���Ƶpacket
            dePacket->stream_index = audioStream->index; //���֮ǰ����Ƶpacket������ҪѰ���µ���Ƶpacket�����֮ǰ����Ƶpacket�������û�����ҪѰ���µ���Ƶpacket
            while (dePacket->stream_index != videoStream->index)
            {
                ret = av_read_frame(inFmtCtx, dePacket);
            }
            //�������ݰ�
            ret = avcodec_send_packet(deVideoCodecCtx, dePacket);
            if (ret < 0)
            {
                av_log_error("Error submitting a packet for deconding\n");
                return nullptr;
            }
            //������Ƶ֡
            while (ret >= 0)
            {
                ret = avcodec_receive_frame(deVideoCodecCtx, deVideoFrame);
                if (ret < 0)
                {
                    if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                        break;
                    av_log_error("Error during decoding");
                    break;
                }
                curFrameIndex = PtsToFrameIndex(deVideoFrame->pts);
                av_log_info("current frame number:%d,pts:%ld\n", curFrameIndex, deVideoFrame->pts);
                if (curFrameIndex == targetFrameIndex)
                    break;
                av_frame_unref(deVideoFrame);
                if (ret < 0)
                    break;
            }
        }
        RescaleDevideoFrame();
        return enVideoFrame;
    }
}

int AVPlayer::GetAVFrameCount() const
{
    return videoStream->nb_frames;
}

int AVPlayer::GetVideoWidth() const
{
    return videoStream->codecpar->width;
}

int AVPlayer::GetVideoHeight() const
{
    return videoStream->codecpar->height;
}

void AVPlayer::GetInformation()
{
    /*av_log_info("duration:%d", videoStream->duration);*/
    /*DecodePakcet();*/
    /*int vsDuration = av_rescale_q(videoStream->duration, videoStream->time_base, {1,AV_TIME_BASE});
    av_log_info("video stream duration in audiobase:%d",vsDuration);
    int asDuration = av_rescale_q(audioStream->duration, audioStream->time_base, { 1,AV_TIME_BASE });
    av_log_info("audio stream duration in audiobase:%d", asDuration);*/

    // inFmtCtx = videoFmtCtx;
    // dePacket = deVideoPacket;
    // DecodePakcet();
    // isVideo = false;
    // inFmtCtx = audioFmtCtx;
    // dePacket = deAudioPacket;
    // DecodePakcet();

    /*std::thread audioThread([this]() {
            while(isAudioEnd==false)GetNextAudioData();
        });
    std::thread videoThread([this]() {
        while (isVideoEnd == false) GetNextVideoData();
        });
    audioThread.join();
    videoThread.join();*/

    ReadPacket();
}
bool AVPlayer::GetIsPlayerEnd() const
{
    return isVideoEnd && isAudioEnd;
}
const AVFrame *AVPlayer::GetNextFrame(AVFormatContext *fmtCtx, AVCodecContext *codecCtx, AVPacket *packet, AVFrame *deAvFrame)
{
    //首先从解码器中解码已经缓存的packet数据
    ret = avcodec_receive_frame(codecCtx, deAvFrame);
    if (ret >= 0)
        return deAvFrame;
    //没有已缓存的，需要重新取packet
    int streamIndex = -1;
    if (deAvFrame == deAudioFrame)
        streamIndex = getAudioStreamIndex(fmtCtx);
    else if (deAvFrame == deVideoFrame)
        streamIndex = getVideoStreamIndex(fmtCtx);
    packet->stream_index = -1;
    while (packet->stream_index != streamIndex)
    {
        ret = av_read_frame(fmtCtx, packet);
        if (ret < 0)
            return nullptr; //没有audio packet，已经全部读取完成
    }

    //还有packet，从中获取新的frame
    ret = avcodec_send_packet(codecCtx, packet);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return deAvFrame;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(codecCtx, deAvFrame);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                break;
            av_log_error("Error during decoding");
            break;
        }
        int frameIndex = PtsToFrameIndex(deAvFrame->pts);
        // av_log_info("current frame number:%d,pts:%d\n", frameIndex, deAvFrame->pts);
        break; // error prone
        av_frame_unref(deAvFrame);
        if (ret < 0)
            break;
    }
    return deAvFrame;
}
void AVPlayer::ReadPacket()
{
    if (dePacket == nullptr)
        dePacket = AllocAVPacket();
    while (av_read_frame(inFmtCtx, dePacket) >= 0)
    {
        packetsQueueMutex.lock();
        packets.push(dePacket);
        if (dePacket->stream_index == audioStream->index)
        {
            audioPacketsDuration += av_rescale_q(dePacket->duration, audioStream->time_base, {1, AV_TIME_BASE});
        }
        else if (dePacket->stream_index == videoStream->index)
        {
            videoPacketsDuration += av_rescale_q(dePacket->duration, videoStream->time_base, {1, AV_TIME_BASE});
        }
        av_log_info("audio duration:%ld,video duration:%ld", audioPacketsDuration, videoPacketsDuration);
        packetsQueueMutex.unlock();
        av_packet_free(&dePacket);
        dePacket = AllocAVPacket();
        //累计时间超过1s，进行休眠
        if (max(audioPacketsDuration, videoPacketsDuration) > AV_TIME_BASE)
        {
            av_usleep(AV_TIME_BASE);
        }
    }
}
const AVFrame *AVPlayer::GetNextAudioFrame()
{
    const AVFrame *audioFrame = GetNextFrame(audioFmtCtx, deAudioCodecCtx, deAudioPacket, deAudioFrame);
    if (audioFrame == nullptr)
        isAudioEnd = true;
    return audioFrame;
}
const AVFrame *AVPlayer::GetNextVideoFrame()
{
    const AVFrame *videoFrame = GetNextFrame(videoFmtCtx, deVideoCodecCtx, deVideoPacket, deVideoFrame);
    if (videoFrame == nullptr)
        isVideoEnd = true;
    return videoFrame;
}
void *AVPlayer::GetNextAudioData()
{
    audioNowPts = av_rescale_q(av_gettime(), {1, AV_TIME_BASE}, audioStream->time_base) - audioLastTime + audioLastPts;

    if (deAudioFrame->pts <= audioNowPts)
    {
        audioLastPts = deAudioFrame->pts;
        ResampleDeAudioFrame();
        GetNextAudioFrame();
        audioLastTime = av_rescale_q(av_gettime(), {1, AV_TIME_BASE}, audioStream->time_base);
        av_log_error("audio now pts:%d,last pts:%d\n", audioNowPts, audioLastPts);
        return enAudioFrame->data[0];
    }
    else
    {
        // audioLastTime = av_rescale_q(av_gettime(), {1, AV_TIME_BASE}, audioStream->time_base);
        // av_log_info("audio now pts:%d,last pts:%d", audioNowPts, audioLastPts);
        return nullptr;
    }
}
bool AVPlayer::GetIsVideoEnd() const
{
    return isVideoEnd;
}
bool AVPlayer::GetIsAudioEnd() const
{
    return isAudioEnd;
}
void *AVPlayer::GetNextVideoData()
{
    static int frameNumber = 0;
    int64_t videoLastPts = av_rescale_q(deVideoFrame->pts, videoStream->time_base, audioStream->time_base);
    if (videoLastPts == audioLastPts || (videoLastPts < audioLastPts && (audioLastPts - videoLastPts) <= 4410))
    {
        RescaleDevideoFrame();
        GetNextVideoFrame();
        av_log_info("video last pts:%ld,framenumber:%d\n", videoLastPts, ++frameNumber);

        return enVideoFrame->data[0];
    }
    else if (videoLastPts < audioLastPts && (audioLastPts - videoLastPts) > 4410)
    {
        GetNextVideoFrame();
        RescaleDevideoFrame();
        GetNextVideoFrame();
        av_log_info("video last pts:%ld,framenumber:%d\n", videoLastPts, ++frameNumber);
        return enVideoFrame->data[0];
    }
    else
        return nullptr;
}
void AVPlayer::SaveJpeg(const AVFrame *frame, const char *jpegPath)
{
    FILE *dstFile = fopen(jpegPath, "wb");
    fwrite(enVideoFrame->data[0], 1, enVideoFrame->linesize[0] * deHeight, dstFile);
    fclose(dstFile);
}
// feiqi
//  set avforrmatcontext
// AVFormatContext* jpegOutFmtCtx = AllocOutFormatContext(jpegPath);
// jpegOutFmtCtx->oformat = av_guess_format("mjpeg", nullptr, nullptr);
//  set avcodeccontext
// const AVCodec* codec = avcodec_find_encoder(videoStream->codecpar->codec_id);
// if (codec == nullptr)
//{
//     av_log_error("find codec failed\n");
//     return;
// }
// AVCodecContext* enCodecContext = avcodec_alloc_context3(codec);
// enCodecContext->codec_id = videoStream->codecpar->codec_id;
// enCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
// enCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
// enCodecContext->width = frame->width;
// enCodecContext->height = frame->height;
// enCodecContext->time_base = { 1, 25 };
// if (avcodec_open2(enCodecContext, codec, nullptr) < 0)
//{
//     av_log_error("could not open encodec context\n");
//     return;
// }
////
// AddNewStreamToFormat(jpegOutFmtCtx, enCodecContext);
// if (avio_open(&jpegOutFmtCtx->pb, jpegPath, AVIO_FLAG_WRITE) < 0)
//{
//     av_log_error("open file point failed\n");
//     return;
// }
// int ret = avformat_write_header(jpegOutFmtCtx, nullptr);
// if (ret < 0)
//{
//     av_log_error("write header failed\n");
//     return;
// }
//// write to file
// AVPacket* packet = AllocAVPacket();
// ret = avcodec_send_frame(enCodecContext, frame);
// if (ret < 0) {
//     av_log_error("could not send frame to encodec context\n");
//     return;
// }
// avcodec_send_frame(enCodecContext, nullptr);
// while (ret >= 0)
//{
//     ret = avcodec_receive_packet(enCodecContext, packet);
//     if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
//     {
//         av_log_error("eagain averror_eof %d\n", ret);
//         break;
//     }
//
//     if (ret < 0)
//     {
//         av_log_error("enVideoCodecContext receive packet error\n");
//         break;
//     }
//     //av_packet_rescale_ts(enVideoPacket, enVideoCodecCtx->time_base, outVideoStream->time_base);
//     //enVideoPacket->stream_index = outVideoStream->index;
//     packet->stream_index = 0;
//     av_write_image_line
//         av_write(jpegOutFmtCtx, packet);
//     av_packet_unref(packet);
//     if (ret < 0)
//         break;
// }
// av_write_trailer(jpegOutFmtCtx);
//// free
// avcodec_close(enCodecContext);
// avio_close(jpegOutFmtCtx->pb);
// avformat_free_context(jpegOutFmtCtx);
// av_packet_free(&packet);
// avcodec_free_context(&enCodecContext);
