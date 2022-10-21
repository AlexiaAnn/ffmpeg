#include "AVFileContext.h"
#include <queue>
#include <thread>
#include <mutex>
class AVPlayer : public AVFileContext
{
private:
    AVFormatContext* videoFmtCtx;
    AVFormatContext* audioFmtCtx;
    AVStream* audioStream;
    AVStream *videoStream;
    AVPacket* deVideoPacket;
    AVPacket* deAudioPacket;
    int audioLastPts,audioLastTime,audioNowPts;
    int curFrameIndex;
    int ret;
    bool isVideo;
    bool isVideoEnd,isAudioEnd;

    //-----------
    std::queue<AVPacket*> packets;
    int64_t packetQueueDuration;
    int64_t audioPacketsDuration;
    int64_t videoPacketsDuration;
    std::mutex packetsQueueMutex;

private:
    inline int PtsToFrameIndex(int64_t pts);
    inline int64_t FrameIndexToPts(int frameIndex);
protected:
    void DealVideoPacket() override;
    void DealAudioPacket() override;
    void DealDeAudioFrame() override;
    void DealDeVideoFrame() override;
    inline const AVFrame *GetNextFrame(AVFormatContext *fmtCtx, AVCodecContext *codecCtx, AVPacket *packet, AVFrame *deAvFrame);
    void ReadPacket();
public:
    AVPlayer();
    AVPlayer(const char *srcFilePath);
    const AVFrame *SeekFrameByFrameIndex(int frameIndex);
    const AVFrame* GetFrameAtTargetPercent(float percent);
    int GetAVFrameCount() const;
    int GetVideoWidth() const;
    int GetVideoHeight() const;
    void GetInformation();
    bool GetIsPlayerEnd() const;
    const AVFrame* GetNextVideoFrame();
    const AVFrame* GetNextAudioFrame();
    void* GetNextVideoData();
    void* GetNextAudioData();
    bool GetIsVideoEnd() const;
    bool GetIsAudioEnd() const;
    void SaveJpeg(const AVFrame *frame, const char *jpegPath);
};