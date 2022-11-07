#pragma once
#include "VideoEnCodecConfigBase.h"


#define DEFAULTCRFMAXVALUE 51
#define DEFAULTCRFMIN 18
#define DEFAULTCRFMAX 28
#define DEFAULTBITRATEPERCENT 0.2
enum PresetLevel {
    ULTRAFAST = 0,
    SUPERFAST,
    VERYFAST,
    FASTER,
    FAST,
    MEDIUM,
    SLOW,
    SLOWER,
    VERYSLOW
};
class Libx264EncodecConfig :
    public VideoEnCodecConfigBase
{
private:
    static const std::string presetLevels[9];
    
    float bitRatePercent=0.2;
    int crfMin=18;
    int crfMax=28;
    PresetLevel level=ULTRAFAST;
public:
    Libx264EncodecConfig(int width,int height,int fps,EncodeName encodeName,
                         float bitRatePercent,int crfMin,int crfMax,enum PresetLevel level);
    AVCodecContext* GetContext() override;
};

