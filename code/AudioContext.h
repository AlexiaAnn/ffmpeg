#pragma once
#include "IncludeFFmpeg.h"
#include "util.h"
#define DEFAULTAUDIOCODECID AV_CODEC_ID_MP3
class AudioContext
{
protected:
    AVCodecContext *enAudioCodecCtx;
    AVFrame *deAudioFrame;
    AVFrame *enAudioFrame;
    AVPacket *enAudioPacket;
    SwrContext *swrCtx;
    int maxDstNbSamples;
    int pts;
    int ret;
    AVSampleFormat srcSampleFormat; // todo 构造函数初始化
    int srcSampleRate;              // todo 构造函数初始化
    AVChannelLayout srcChlayout;    // todo 构造函数初始化
    AVFormatContext *outAudioFmtCtx;
    AVStream *outAudioStream;

protected:
    AudioContext();
    void ResampleDeAudioFrame();
    void EncodeAudioFrame();
    void InitEnAudioFrame(int dstNbSamples);
    AVCodecContext *OpenAudioEncodecContext(AVCodecID enCodecId);
    SwrContext *InitSwrContext(AVCodecContext *deCodecCtx, AVCodecContext *enCodecCtx);
    SwrContext *InitSwrContext(int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout, AVCodecContext *enCodecCtx);
    SwrContext* InitSwrContext(AVCodecContext* deCodecCtx,int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout);
    bool VariableCheck();

public:
    AudioContext(AVCodecID dstCodecId);
    AudioContext(int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout);
    AudioContext(int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout, AVCodecID codecId);
    void ResetAudioCodecId(AVCodecID codecId);
    void FlushEnAudioCodecBuffer();
    // write to file
    void WriteAudioPreparition(const char *dstFilePath);
    void WriteAudioToFile(AVFrame* deAudioFrame);
    void WriteAudioToFile(void *data, int length);
    void WriteAudioToFile();
    void WriteAudioTrailer();
    virtual ~AudioContext();
};