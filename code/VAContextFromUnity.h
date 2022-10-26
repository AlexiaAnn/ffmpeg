#include "AudioContext.h"
#include "VideoContext.h"
#include <vector>

class VAContextFromUnity : public AudioContext, public VideoContext
{
private:
    AVFormatContext *outFmtCtx;
    bool isAudio;
    AVStream *outAudioStream;
    AVStream *outVideoStream;

public:
    VAContextFromUnity() = delete;
    VAContextFromUnity(const char *srcFilePth, const char *dstFilePath);
    VAContextFromUnity(const char *dstFilePath,
                       int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout,
                       AVPixelFormat dePixFormat, int fps, int deWidth, int deHeight);
    void MuexFileToMp4File(const char *srcFilePath, const char *dstFilePath);
    void WriteToFilePrepare(const char *dstFilePath);
    void WriteToFileEnd();
    void DealAudioPacket(AVPacket *packet) override;
    void DealVideoPacket(AVPacket *packet) override;
    void EncodeAudioFrame() override;
    void EncodeVideoFrame() override;
    void WriteAudioFrame(void *data, int length);
    void WriteVideoFrame(void *data);
};