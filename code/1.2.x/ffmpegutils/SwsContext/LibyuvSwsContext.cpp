#include "LibyuvSwsContext.h"

bool LibyuvSwsContext::RescaleVideoFrame(AVFrame *deVideoFrame, AVFrame *enVideoFrame)
{
    return false;
}
bool LibyuvSwsContext::RescaleVideoFrame(AVFrame *deVideoFrame, EnCodecVideoContext &codeCont)
{
    AVFrame *enVideoFrame = codeCont.GetEncodecFrame();
    libyuv::ABGRToI420(deVideoFrame->data[0], deVideoFrame->linesize[0],
                       enVideoFrame->data[0], enVideoFrame->linesize[0],
                       enVideoFrame->data[1], enVideoFrame->linesize[1],
                       enVideoFrame->data[2], enVideoFrame->linesize[2],
                       enVideoFrame->width, enVideoFrame->height); // error prone (w+1)/2 => enVideoFrame->linesize[1]
    return true;
}
int LibyuvSwsContext::GetResult() const
{
    return ret;
}