#pragma once
#include "ReadFileBase.h"
#include "ffmpegutils/AVSwrContext.h"
#include <thread>
#define DEFAULTPOINTNUMBER 4000
#define INPUT_SAMPLERATE 44100
#define INPUT_FORMAT AV_SAMPLE_FMT_FLTP
class AudioWaveA : public ReadFileBase
{
private:
	AVStream *audioStream = nullptr;
	AVSwrContext *swrCont = nullptr;
	AVFrame *enAudioFrame = nullptr;
	std::thread mThread;

private:
	bool isNeedResample(AVSampleFormat format, int channelCount) const;

public:
	AudioWaveA(const char *srcFilePath);
	int GetMetaDataLength() const;
	float GetSecondsOfDuration() const;
	int GetAudioSampleRate() const;
	int GetAudioBitRate() const;
	void GetMetaData(float *result);
	void GetAudioInformation();
};
