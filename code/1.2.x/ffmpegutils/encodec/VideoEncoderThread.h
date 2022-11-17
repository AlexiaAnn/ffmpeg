#pragma once
#ifdef WINDOWS
#include "EnCodecVideoContext.h"
#include "../utils/QueueThread.h"
#endif // WINDOWS
#ifdef ANDROID
#include "EnCodecVideoContext.h"
#include "utils/QueueThread.h"
#endif
class VideoEncoderThread :public EnCodecVideoContext {
private:
	std::thread mThread;
	QueueThread mQueue;
	bool mIsEnd = false;
	OutFormatContext* mOFormat=nullptr;
	AVStream* mVideoStream=nullptr;
public:
	VideoEncoderThread(AVCodecID encodeId, int width, int height, int fps,
		float bitRatePercent, int crfMin, int crfMax, int presetLevel);
	VideoEncoderThread(EncodeName encodeName, int width, int height, int fps,
		float bitRatePercent, int crfMin, int crfMax, int presetLevel);
	bool EncodeFrame(int id, AVFrame* frame);
	void ThreadStart(OutFormatContext* format, AVStream* stream);
	bool FlushBuffer(OutFormatContext& outFmtCont, AVStream* outStream) override;
	~VideoEncoderThread();
};