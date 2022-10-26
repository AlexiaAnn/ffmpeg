#pragma once
#include "util.h"
class InFormatContext {
private:
    AVFormatContext* inFmtContext;
    AVPacket* dePacket;
    int ret;
public:
    InFormatContext(const char* srcFilePath);
    ~InFormatContext();
    int GetResult() const;

    void DumpFileInfo() const;

    //stream
    AVStream* GetVideoStream() const;
    AVStream* GetAudioStream() const;
    AVStream* GetVideoStreamByWhile() const;
    AVStream* GetAudioStreamByWhile() const;
    //get packet
    AVPacket* GetNextPacket();
    //formatcontext
    AVFormatContext* GetInFormatContext() const;
};

