#include "SeekComponent.h"
int SeekComponent::PtsToFrameIndex(int64_t pts)
{
	return av_rescale_q(pts, videoStream->time_base, { videoStream->r_frame_rate.den,videoStream->r_frame_rate.num });
}
int64_t SeekComponent::FrameIndexToPts(int frameIndex)
{
	return av_rescale_q(frameIndex, { videoStream->r_frame_rate.den,videoStream->r_frame_rate.num }, videoStream->time_base);
}

SeekComponent::SeekComponent(const char* srcFilePath):FileContextBase(srcFilePath),
VideoFileContext(AV_PIX_FMT_YUV420P, AV_CODEC_ID_GIF,
    inFmtCtx->streams[getVideoStreamIndex()]->codecpar->width,
    inFmtCtx->streams[getVideoStreamIndex()]->codecpar->height,
    inFmtCtx->streams[getVideoStreamIndex()]->r_frame_rate.num,
    inFmtCtx->streams[getVideoStreamIndex()]->codecpar->width,
    inFmtCtx->streams[getVideoStreamIndex()]->codecpar->height),
    curFrameIndex(-1)
{
    if (inFmtCtx == nullptr) {
        ret = -1;
        return;
    }
    VideoFileContext::VariableCheck(srcFilePath);
    ret = getVideoStreamIndex();
    if (ret == AVERROR_STREAM_NOT_FOUND) {
        ret = -1;
        return;
    }
	videoStream = inFmtCtx->streams[ret];
    if (videoStream->nb_frames <= 0) {
        frameNumber = av_rescale_q(inFmtCtx->duration, { 1,AV_TIME_BASE }, {videoStream->r_frame_rate.den,videoStream->r_frame_rate.num});
        av_log_info("frame count of video is %d\n",frameNumber);
    }
    else {
        frameNumber = videoStream->nb_frames;
        av_log_info("frame count of video is %d\n", frameNumber);
    }
    swsCtx = InitSwsContext(deVideoCodecCtx, enVideoCodecCtx);
}

int SeekComponent::GetVideoWidth() const
{
    return videoStream->codecpar->width;
}

int SeekComponent::GetVideoHeight() const
{
    return videoStream->codecpar->height;
}

double SeekComponent::GetDuration() const
{
    return (double)inFmtCtx->duration/AV_TIME_BASE;
}

void SeekComponent::GetFrameDataByPercent(float percent, void* data,int length)
{
    AVFrame* targetFrame = SeekFrameByPercent(percent);
    av_log_info("curFrameIndex:%d\n", curFrameIndex);
    if (targetFrame == nullptr) {
        av_log_error("cant find the target frame");
        return;
    }
    if (data == nullptr) {
        av_log_error("data buffer is nullptr,memory copy error\n");
        return;
    }
    memcpy(data,targetFrame->data[0],length);
    
}

AVFrame* SeekComponent::SeekFrameByPercent(float percent)
{
    int targetFrameIndex = int(percent * frameNumber);
    if (targetFrameIndex <0) return nullptr;
    if (targetFrameIndex == curFrameIndex) return enVideoFrame;
    if (targetFrameIndex - curFrameIndex > 0 && targetFrameIndex - curFrameIndex < videoStream->r_frame_rate.num) {
        while (targetFrameIndex != curFrameIndex) {
            if (GetNextFrame() == nullptr)break;
        }  
    }
    else {
        SeekFrameByFrameIndex(targetFrameIndex);
        while (targetFrameIndex != curFrameIndex) {
            if (GetNextFrame() == nullptr)break;
        }
    }
    RescaleDevideoFrame();
    av_frame_unref(deVideoFrame);
    return enVideoFrame;
}

int SeekComponent::GetRet() const
{
    return ret;
}

SeekComponent::~SeekComponent()
{
    if (deVideoCodecCtx != nullptr) {
        avcodec_free_context(&deVideoCodecCtx);
    }
}

void SeekComponent::SeekFrameByFrameIndex(int frameIndex)
{
	int64_t ts = FrameIndexToPts(frameIndex);
	int ret = av_seek_frame(inFmtCtx, videoStream->index, ts, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        av_log_info("av_seek_frame failed\n");
    }
}

AVFrame* SeekComponent::GetNextFrame()
{
    //首先从解码器中解码已经缓存的packet数据
    ret = avcodec_receive_frame(deVideoCodecCtx, deVideoFrame);
    if (ret >= 0)
        goto the_end;
    //没有已缓存的，需要重新取packet
    dePacket->stream_index = -1;
    retry:
    do
    {
        ret = av_read_frame(inFmtCtx, dePacket);
        if (ret < 0)
            return nullptr; //没有packet，已经全部读取完成
    } while (dePacket->stream_index != videoStream->index);

    //还有packet，从中获取新的frame
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
                goto retry;
            av_log_error("Error during decoding");
            break;
        }
        break; // error prone
        if (ret < 0)
            break;
    }
the_end:
    curFrameIndex = PtsToFrameIndex(deVideoFrame->pts);
    av_log_info("cur frame index:%d\n",curFrameIndex);
    return deVideoFrame;
}
