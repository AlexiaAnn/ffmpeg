#include "ContextBase.h"
#include <vector>
class VAContext : public ContextBase
{
protected:
    int videoPacketCount;
    int audioPacketCount;
    std::vector<AVPacket *> audioPackets;
    std::vector<AVPacket *> videoPackets;
    AVFormatContext *outFmtCtx;
    AVStream *inAudioStream;
    AVStream *inVideoStream;
    AVStream *outAudioStream;
    AVStream *outVideoStream;

    OutputStream videoSt, audioSt;
    int maxDstNbSamples;

protected:
    void DealAudioPacket(AVPacket *packet) override;
    void DealVideoPacket(AVPacket *packet) override;
    void WriteToFilePrepare(const char *dstFilepath);
    void WriteToFileEnd();
    void WriteToFile();
    void InitOutFormatContextByFileName(const char *dstFilePath);
    void SetVideoOutputStreamStruct();
    void SetVideoOutputStreamStruct(int fps, int width, int height);
    void SetAudioOutputStreamStruct();
    void RescaleDeVideoFrame();
    void EncodeVideoFrame();
    void ResampleDeAudioFrame();
    void EncodeAudioFrame();
    void OutputStreamFree(OutputStream& ost);

public:
    VAContext() = delete;
    VAContext(const char *srcFilePath);
    VAContext(const char *srcFilePath, const char *dstFilepath);
    VAContext(const char *srcFilePath, const char *dstFilepath, int fps, int width, int height);
    void MuxeVideoAndAudio(const char *dstFilepath);
};