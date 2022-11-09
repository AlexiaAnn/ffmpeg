#include "AudioWaveA.h"

bool AudioWaveA::isNeedResample(AVSampleFormat format, int channelCount) const
{
    if (channelCount == 1)
    {
        if (format == AV_SAMPLE_FMT_FLT || format == AV_SAMPLE_FMT_FLTP)
            return false;
        else
            return true;
    }
    else
        return false;
}

AudioWaveA::AudioWaveA(const char *srcFilePath) : ReadFileBase(srcFilePath, AVMEDIA_TYPE_AUDIO), swrCont(nullptr), enAudioFrame(nullptr)
{
    if (ret < 0)
    {
        ret = -1;
        av_log_error("AudioWave initialize failed\n");
        return;
    }
    audioStream = inFmtCont->GetAudioStream();
    if (audioStream == nullptr)
    {
        ret = -1;
        av_log_error("src file has no audio stream\n");
        return;
    }
    AVSampleFormat format = (AVSampleFormat)audioStream->codecpar->format;
    int channelCount = audioStream->codecpar->ch_layout.nb_channels;
    if (isNeedResample(format, channelCount))
    {
        int sampleRate = audioStream->codecpar->sample_rate;
        swrCont = new AVSwrContext(sampleRate, format, audioStream->codecpar->ch_layout,
                                   sampleRate, AV_SAMPLE_FMT_FLTP, MONOLAYOUT);
        if (swrCont->GetResult() < 0)
        {
            ret = -1;
            av_log_error("AudioWave Context initialize failed\n");
            return;
        }
        enAudioFrame = AllocAVFrame();
        if (enAudioFrame == nullptr)
        {
            ret = -1;
            av_log_error("AudioWave Context initialize failed\n");
            return;
        }
    }
    ret = 0;
}

int AudioWaveA::GetMetaDataLength() const
{
    return DEFAULTPOINTNUMBER;
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

void AudioWaveA::GetMetaData(float *result)
{

    AVFrame *frame = nullptr;
    int samples = int(GetSecondsOfDuration() * audioStream->codecpar->sample_rate);
    const int interval = samples / DEFAULTPOINTNUMBER;
    int i = 0;
    int resultIndex = 0;
    while ((frame = deCodeCont->GetNextFrame(*inFmtCont)) != nullptr)
    {
        if (swrCont != nullptr)
        {
            swrCont->ResampleAudioFrame(frame, enAudioFrame);
            frame = enAudioFrame;
        }
        float *data = (float *)frame->data[0];
        for (; i < frame->nb_samples && resultIndex < GetMetaDataLength(); i += interval, ++resultIndex)
        {
            *(result + resultIndex) = *(data + i);
        }
        if (resultIndex >= GetMetaDataLength())
            break;
        i -= frame->nb_samples;
    }
}

void AudioWaveA::GetAudioInformation()
{
    // todo
    float *data = new float[GetMetaDataLength()];
    GetMetaData(data);
    delete[] data;
}
