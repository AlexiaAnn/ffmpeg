
#ifdef ANDROID
#include "LibyuvSwsContext.h"

bool LibyuvSwsContext::RescaleVideoFrame(AVFrame* deVideoFrame, AVFrame* enVideoFrame)
{
    start = clock();
    libyuv::ABGRToI420(deVideoFrame->data[0], deVideoFrame->linesize[0],
        enVideoFrame->data[0], enVideoFrame->linesize[0],
        enVideoFrame->data[1], enVideoFrame->linesize[1],
        enVideoFrame->data[2], enVideoFrame->linesize[2],
        enVideoFrame->width, -enVideoFrame->height); // error prone (w+1)/2 => enVideoFrame->linesize[1]
    end = clock();
    swsTime += float(end - start) / CLOCKS_PER_SEC;
    frameCount += 1;
    return true;
}
bool LibyuvSwsContext::RescaleVideoFrame(AVFrame* deVideoFrame, EnCodecVideoContext& codeCont)
{
    start = clock();
    AVFrame* enVideoFrame = codeCont.GetEncodecFrame();
    libyuv::ABGRToI420(deVideoFrame->data[0], deVideoFrame->linesize[0],
        enVideoFrame->data[0], enVideoFrame->linesize[0],
        enVideoFrame->data[1], enVideoFrame->linesize[1],
        enVideoFrame->data[2], enVideoFrame->linesize[2],
        enVideoFrame->width, -enVideoFrame->height); // error prone (w+1)/2 => enVideoFrame->linesize[1]
    end = clock();
    swsTime += float(end - start) / CLOCKS_PER_SEC;
    frameCount += 1;
    return true;
}
int LibyuvSwsContext::GetResult() const
{
    return ret;
}
void LibyuvSwsContext::GetTimeInfo() const
{
    av_log_info("test time=>[sws time:%fs,avg:%fs]", swsTime, swsTime / frameCount);
}
#endif // ANDROID

