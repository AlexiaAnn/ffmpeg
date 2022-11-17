#pragma once
#ifdef WINDOWS
#include "EnCodecContext.h"
#include "../ffmpegutils/codec_configs/CodecConfigManager.h"
#endif // WINDOWS
#ifdef ANDROID
#include "EnCodecContext.h"
#include "ffmpegutils/codec_configs/CodecConfigManager.h"
#endif // ANDROID

#define DEFAULTCODECID AV_CODEC_ID_H264
#define DEFAULTCRFMAXVALUE 51
#define DEFAULTCRFMIN 18
#define DEFAULTCRFMAX 28
#define DEFAULTBITRATEPERCENT 0.2
class EnCodecVideoContext : public EnCodecContext
{
private:
    static const std::string presetLevels[9];

    //≤‚ ‘±‰¡ø
    float codeTime = 0;
    float writePktTime = 0;
    clock_t start, end;
protected:
    AVCodecContext *OpenEncodecContext(AVCodecID enCodecid, int width, int height, int fps, float bitRatePercent, int crfMin, int crfMax, int presetLevel);
    AVFrame *CreateVideoFrame(const AVCodecContext *codeCont);
    int inFrameCount = 0;

public:
    EnCodecVideoContext();
    EnCodecVideoContext(AVCodecID encodeId, int width, int height, int fps,
                        float bitRatePercent, int crfMin, int crfMax, int presetLevel);
    EnCodecVideoContext(EncodeName encodeName, int width, int height, int fps,
                        float bitRatePercent, int crfMin, int crfMax, int presetLevel);

    bool EncodeFrame(OutFormatContext &outFmtCont, AVStream *outStream) override;
    bool EncodeFrame(OutFormatContext &outFmtCont, AVStream *outStream, AVFrame *enFrame) override;
    bool FlushBuffer(OutFormatContext &outFmtCont, AVStream *outStream) override;
    AVFrame *GetEncodecFrame() const;
    void GetTimeInfo() const;
    virtual ~EnCodecVideoContext();
};
