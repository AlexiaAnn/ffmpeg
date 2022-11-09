#pragma once
#include "SwsContextBase.h"
#include "convert.h"
class LibyuvSwsContext : public SwsContextBase
{
private:
    int ret = 0;

public:
    LibyuvSwsContext() : ret(0) {}
    bool RescaleVideoFrame(AVFrame *deVideoFrame, AVFrame *enVideoFrame) override;
    bool RescaleVideoFrame(AVFrame *deVideoFrame, EnCodecVideoContext &codeCont) override;
    int GetResult() const override;
    ~LibyuvSwsContext(){};
};