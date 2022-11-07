#include "SeekVideo.h"

inline int SeekVideo::PtsToFrameIndex(int64_t pts)
{
    return av_rescale_q(pts, videoStream->time_base, { videoStream->avg_frame_rate.den,videoStream->avg_frame_rate.num });
}

inline int64_t SeekVideo::FrameIndexToPts(int frameIndex)
{
    return av_rescale_q(frameIndex, { videoStream->avg_frame_rate.den,videoStream->avg_frame_rate.num }, videoStream->time_base);
}

AVFrame* SeekVideo::SeekFrameByPercent(float percent)
{
    AVFrame* deFrame = nullptr;
    int targetFrameIndex = int(percent * frameNumber);
    
    if (targetFrameIndex < 0) return nullptr;
    if (targetFrameIndex == curFrameIndex) return swsVideoFrame;
    if (targetFrameIndex - curFrameIndex > 0 && targetFrameIndex - curFrameIndex < fps) {
        //av_log_info("cur_frame_index:%d,target_frame_index:%d,next frame\n", curFrameIndex, targetFrameIndex);
        while (targetFrameIndex > curFrameIndex) {
            if ((deFrame= GetNextFrame()) == nullptr)break;
        }
    }
    else {
        //av_log_info("cur_frame_index:%d,target_frame_index:%d,seek frame\n", curFrameIndex, targetFrameIndex);
        SeekFrameByFrameIndex(targetFrameIndex);
        curFrameIndex = targetFrameIndex - 1;
        while (targetFrameIndex > curFrameIndex) {
            if ((deFrame = GetNextFrame()) == nullptr)break;
        }
    }
    if (deFrame == nullptr) return nullptr;
    swsContPointer->RescaleVideoFrame(deFrame,swsVideoFrame);
    //av_frame_unref(deFrame);
    return swsVideoFrame;
}

AVFrame* SeekVideo::GetNextFrame()
{
    //首先从解码器中解码已经缓存的packet数据
    AVFrame* frame = nullptr;
    frame = deCodecont->GetNextFrame(*inFmtCont);
    if (frame == nullptr) return nullptr;
    /*while (frame == nullptr) {
        frame = deCodecont->GetReceiveFrame();
        if (frame == nullptr) {
            AVPacket* packet;
            while (true) {
                packet = inFmtCont->GetNextPacket();
                if (packet == nullptr) return nullptr;
                else if (packet->stream_index != videoStream->index) continue;
                else {
                    deCodecont->SendPacket(packet);
                    break;
                }
            }

        }
    }*/
    curFrameIndex = PtsToFrameIndex(frame->pts);
    //av_log_info("cur frame index:%d\n",curFrameIndex);
    return frame;
}

void SeekVideo::SeekFrameByFrameIndex(int frameIndex)
{
    int64_t ts = FrameIndexToPts(frameIndex);
    int ret = av_seek_frame(inFmtCont->GetInFormatContext(), videoStream->index, ts, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        av_log_info("av_seek_frame failed\n");
    }
}

SeekVideo::SeekVideo():inFmtCont(nullptr),deCodecont(nullptr),
                       videoStream(nullptr),ret(-1),
                       curFrameIndex(-1),swsContPointer(nullptr),
                       frameNumber(0),fps(0)
{
}

SeekVideo::SeekVideo(const char* srcFilePath):curFrameIndex(-1),ret(0), frameNumber(0), fps(0)
{
    inFmtCont = new InFormatContext(srcFilePath);
    if (inFmtCont->GetResult()<0) goto end;
    videoStream = inFmtCont->GetVideoStreamByWhile();
    if (videoStream == nullptr) goto end;
    deCodecont = new DeCodecContext(videoStream);
    if (deCodecont->GetResult()<0) goto end;
    swsContPointer = new AVSwsContext(AV_PIX_FMT_YUV420P, GetVideoWidth(), GetVideoHeight(), AV_PIX_FMT_RGBA,GetVideoWidth(),GetVideoHeight());
    if (swsContPointer->GetResult()<0) goto end;
    swsVideoFrame = CreateVideoFrame(AV_PIX_FMT_RGBA,GetVideoWidth(),GetVideoHeight());
    if (swsVideoFrame==nullptr) goto end;

    if (videoStream->nb_frames <= 0) {
        frameNumber = av_rescale_q(inFmtCont->GetInFormatContext()->duration, 
                                   { 1,AV_TIME_BASE }, 
                                   { videoStream->avg_frame_rate.den,videoStream->avg_frame_rate.num });
        av_log_info("nb_frames<=0,frame count of video is %d\n", frameNumber);
    }
    else {
        frameNumber = videoStream->nb_frames;
        av_log_info("nb_frames>0,frame count of video is %d\n", frameNumber);
    }
    fps = videoStream->avg_frame_rate.num / videoStream->avg_frame_rate.den;
    return;
end:
    ret = -1;
    return;
}

int SeekVideo::GetVideoWidth() const
{
    return videoStream->codecpar->width;
}

int SeekVideo::GetVideoHeight() const
{
    return videoStream->codecpar->height;
}

float SeekVideo::GetDuration() const
{
    return (float)inFmtCont->GetInFormatContext()->duration / AV_TIME_BASE;
}

void SeekVideo::GetFrameDataByPercent(float percent, void* data, int length)
{
    AVFrame* targetFrame = SeekFrameByPercent(percent);
    //av_log_info("curFrameIndex:%d\n", curFrameIndex);
    if (targetFrame == nullptr) {
        av_log_error("cant find the target frame\n");
        return;
    }
    if (data == nullptr) {
        av_log_error("data buffer is nullptr,memory copy error\n");
        return;
    }
    memcpy(data, targetFrame->data[0], length);
}

int SeekVideo::GetResult() const
{
    return ret;
}

void SeekVideo::GetSomeInformation() const
{
    /*av_log_info("avg_frame_rate=>num:%d,den:%d\n",videoStream->avg_frame_rate.num, videoStream->avg_frame_rate.den);
    av_log_info("avg fps:%d\n", videoStream->avg_frame_rate.num/videoStream->avg_frame_rate.den);
    av_log_info("r_frame_rate=>num:%d,den:%d\n", videoStream->r_frame_rate.num, videoStream->r_frame_rate.den);
    av_log_info("r fps:%d\n", videoStream->r_frame_rate.num / videoStream->r_frame_rate.den);*/
    while ((deCodecont->GetNextFrame(*inFmtCont)) != nullptr);
}

SeekVideo::~SeekVideo()
{
    delete inFmtCont;
    inFmtCont = nullptr;
    delete deCodecont;
    deCodecont = nullptr;
}
