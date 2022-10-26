#pragma once
#include "AudioFileContext.h"
#define DEFAULTPOINTNUMBER 4000
#define INPUT_SAMPLERATE     44100
#define INPUT_FORMAT         AV_SAMPLE_FMT_FLTP
#define INPUT_CHANNEL_LAYOUT (AVChannelLayout)AV_CHANNEL_LAYOUT_5POINT0

#define VOLUME_VAL 0.90
class AudioWave :public AudioFileContext {
private:
	AVStream* audioStream;
	int ret;
public:
	AudioWave(const char* srcFilePath);
	int GetMetaDataLength() const;
	double GetSecondsOfDuration() const;
	int GetAudioSampleRate() const;
	int GetAudioBitRate() const;
	void GetMetaData(float* result);
	int GetRet() const;
	void GetAudioInformation();
	~AudioWave();
};
