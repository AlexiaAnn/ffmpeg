#pragma once
#include "EnCodecContext.h"
#include "ffmpegutils/OutFormatContext.h"
#include "ffmpegutils/codec_configs/CodecConfigManager.h"
#define DEFAULTCODECID AV_CODEC_ID_H264
#define DEFAULTCRFMAXVALUE 51
#define DEFAULTCRFMIN 18
#define DEFAULTCRFMAX 28
#define DEFAULTBITRATEPERCENT 0.2
class EnCodecVideoContext : public EnCodecContext
{
private:
    static const std::string presetLevels[9];

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
    ~EnCodecVideoContext();
};
