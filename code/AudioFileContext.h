#pragma once
#include "FileContextBase.h"
#include "AudioContext.h"
class AudioFileContext : virtual public FileContextBase, public AudioContext
{
protected:
    AVCodecContext *deAudioCodecCtx;
    int ret;

protected:
    void DealAudioPacket() override;
    virtual void DealDeAudioFrame();
    bool VariableCheck(const char *srcFilePath);
    AVFrame* GetNextAudioFrame();
public:
    AudioFileContext();
    AudioFileContext(AVCodecID codecId);
    bool ExtractAudioToFile(const char *srcFilePath, const char *dstFilePath);
};