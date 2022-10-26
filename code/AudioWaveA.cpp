#include "AudioWaveA.h"

bool AudioWaveA::isNeedResample(AVSampleFormat format, int channelCount) const
{
    if (channelCount == 1) {
        if (format == AV_SAMPLE_FMT_FLT || format == AV_SAMPLE_FMT_FLTP) return true;
        else return false;
    }
    else return true;
}

AudioWaveA::AudioWaveA(const char* srcFilePath):ReadFileBase(srcFilePath,AVMEDIA_TYPE_AUDIO),swrCont(nullptr),enAudioFrame(nullptr)
{
    if (ret < 0) {
        ret = -1;
        av_log_error("AudioWave initialize failed\n");
        return;
    }
    audioStream = inFmtCont->GetAudioStream();
    if (audioStream == nullptr) {
        ret = -1;
        av_log_error("src file has no audio stream\n");
        return;
    }
    AVSampleFormat format = (AVSampleFormat)audioStream->codecpar->format;
    int channelCount = audioStream->codecpar->ch_layout.nb_channels;
    if (isNeedResample(format, channelCount)) {
        int sampleRate = audioStream->codecpar->sample_rate;
        swrCont = new AVSwrContext(sampleRate,format,audioStream->codecpar->ch_layout,
                                   sampleRate,AV_SAMPLE_FMT_FLTP,MONOLAYOUT);
        if (swrCont->GetResult() < 0) {
            ret = -1;
            av_log_error("AudioWave Context initialize failed\n");
            return;
        }
        enAudioFrame = AllocAVFrame();
        if (enAudioFrame == nullptr) {
            ret = -1;
            av_log_error("AudioWave Context initialize failed\n");
            return;
        }
    }
}

int AudioWaveA::GetMetaDataLength() const
{
    return DEFAULTPOINTNUMBER + 1;
}

float AudioWaveA::GetSecondsOfDuration() const
{
    return (float)inFmtCont->GetInFormatContext()->duration / (float)AV_TIME_BASE;
}

int AudioWaveA::GetAudioSampleRate() const
{
    return audioStream->codecpar->sample_rate;
}

int AudioWaveA::GetAudioBitRate() const
{
    return audioStream->codecpar->bit_rate;
}

void AudioWaveA::GetMetaData(float* result)
{
    AVFrame* frame = nullptr;
    int samples = int(GetSecondsOfDuration() * audioStream->codecpar->sample_rate);
    const int interval = samples / DEFAULTPOINTNUMBER;
    int i = 0;
    int resultIndex = 0;
    while (frame = deCodeCont->GetNextFrame(*inFmtCont)) {
        if (swrCont != nullptr) {
            swrCont->ResampleAudioFrame(frame,enAudioFrame);
            frame = enAudioFrame;
        }
        float* data = (float*)frame->data[0];
        for (; i < frame->nb_samples; i += interval, ++resultIndex) {
            *(result + resultIndex) = *(data + i);
        }
        i -= frame->nb_samples;
        *(result + DEFAULTPOINTNUMBER) = *(data + frame->nb_samples - 1);
    }
}

void AudioWaveA::GetAudioInformation()
{
    //todo
    av_log_warning("function is not be implemented\n");
}
