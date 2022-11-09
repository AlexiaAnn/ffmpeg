#pragma once
#include "VideoEnCodecConfigBase.h"
class EnMediaCodecConfig : public VideoEnCodecConfigBase
{
private:
    int in_timeout = 1000;
    int out_timeout = 1000;
    int in_timeout_times = 3;
    int out_timeout_times = 3;
    float bitRatePercent = 0.2;

public:
    EnMediaCodecConfig(int width, int height, int fps, EncodeName encodeName,
                       float bitRatePercent,
                       int in_timeout, int out_timtout, int in_timeout_times, int out_timeout_times);
    AVCodecContext *GetContext() override;
    ~EnMediaCodecConfig();
};
