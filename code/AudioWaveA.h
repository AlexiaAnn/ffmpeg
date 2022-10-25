#pragma once
#include "ReadFileBase.h"
#include "AVSwrContext.h"
#define DEFAULTPOINTNUMBER 4000
#define INPUT_SAMPLERATE     44100
#define INPUT_FORMAT         AV_SAMPLE_FMT_FLTP
class AudioWaveA:public ReadFileBase
{
private:
	AVStream* audioStream;
	AVSwrContext* swrCont;
	AVFrame* enAudioFrame;
private:
	bool isNeedResample(AVSampleFormat format, int channelCount) const;
public:
	AudioWaveA(const char* srcFilePath);
	int GetMetaDataLength() const;
	float GetSecondsOfDuration() const;
	int GetAudioSampleRate() const;
	int GetAudioBitRate() const;
	void GetMetaData(float* result);
	void GetAudioInformation();
};

