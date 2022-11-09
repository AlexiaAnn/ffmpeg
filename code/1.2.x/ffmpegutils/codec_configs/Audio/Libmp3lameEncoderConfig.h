#pragma once
#include "AudioEncoderConfigBase.h"
class Libmp3lameEncoderConfig :
    public AudioEncoderConfigBase
{
public:
    Libmp3lameEncoderConfig(AudioEncodeName encodeName);
    AVCodecContext* GetContext() override;
    ~Libmp3lameEncoderConfig();
};

