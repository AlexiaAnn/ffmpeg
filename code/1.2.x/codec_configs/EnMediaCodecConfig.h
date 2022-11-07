#pragma once
#include "VideoEnCodecConfigBase.h"
class EnMediaCodecConfig :
    public VideoEnCodecConfigBase
{
private:
    int in_timeout = 10000;
    int out_timeout = 8000;
    int in_timeout_times = 3;
    int out_timeout_times = 1;
public:
    EnMediaCodecConfig(int width,int height,int fps,EncodeName encodeName,
                       int in_timeout,int out_timtout, int in_timeout_times,int out_timeout_times);
    AVCodecContext* GetContext() override;
};

