#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif // ANDROID

class InFormatContext
{
private:
    AVFormatContext *inFmtContext = nullptr;
    AVPacket *dePacket = nullptr;
    int ret = 0;

public:
    InFormatContext(const char *srcFilePath);
    ~InFormatContext();
    int GetResult() const;

    void DumpFileInfo() const;

    // stream
    AVStream *GetVideoStream() const;
    AVStream *GetAudioStream() const;
    AVStream *GetVideoStreamByWhile() const;
    AVStream *GetAudioStreamByWhile() const;
    // get packet
    AVPacket *GetNextPacket();
    // formatcontext
    AVFormatContext *GetInFormatContext() const;
};
