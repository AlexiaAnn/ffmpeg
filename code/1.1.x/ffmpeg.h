#pragma once
#define _DLLExport __attribute__((visibility("default"))) //定义该函数的dll
#define Export(type) extern "C" type __attribute__((visibility("default")))
#include "VideoFileContext.h"
#include "AudioFileContext.h"
#include "AVContext.h"
#include "AVPlayer.h"
#include "NewPlayer.h"
#include "AudioWave.h"
#include "SeekComponent.h"
#include <thread>
#define ID_CHECK_RETUREZERO
#define ID_CHECK_NORETURE
VideoFileContext *videoContext = nullptr;
std::thread frameConsumeThread;
AVContext *vaContext = nullptr;
AVPlayer *player = nullptr;
NewPlayer *newPlayer = nullptr;
AudioWave *waveContext = nullptr;

// seekcomponent
std::vector<SeekComponent *> seekCpts;
std::mutex seekVectorMutex;
// audiowave
std::vector<AudioWave *> audioWaves;
std::mutex waveVectorMutex;
Export(void) Mp4ToMp3(char *srcFileName, char *dstFileName);
Export(void) InitCSharpDelegate(void (*Log)(char *message, int iSize), void (*LogError)(char *message, int iSize));
Export(bool) RecordMP4Start(const char *dstFilePath, int width, int height, int fps);
Export(void) WriteMP4Frame(void *dataPtr, int length);
Export(void) RecordMP4End();
Export(bool) RecordAVStartWithLogToUnity(const char *dstFilePath, int sampleRate, int channelCount, int width, int height, int fps);
Export(bool) RecordAVStartWithLogToTxt(const char *dstFilePath, int sampleRate, int channelCount, int width, int height, int fps);
Export(bool) RecordAVStart(const char *dstFilePath, int sampleRate, int channelCount, int width, int height, int fps);
Export(void) WriteVideoFrame(void *dataPtr);
Export(void) WriteAudioFrame(void *dataPtr, int length);
Export(void) FlushVideoBuffer();
Export(void) RecordAVEnd();

// player
Export(bool) InitPlayer(char *srcFilePath);
Export(void *) GetRGBAByTargetPercent(float percent);
Export(void *) GetNextRGBAData();
Export(void *) GetNextPCMData();
Export(bool) GetIsVideoEnd();
Export(bool) GetIsAudioEnd();
Export(int) GetAVFileFrameCount();
Export(int) GetVideoWidth();
Export(int) GetVideoHeight();
Export(void) ClosePlayer();

// newplayer
Export(bool) InitNewPlayer(char *srcFilePath);
Export(bool) SetUnityPlayerVideoData(void *data);
Export(bool) SetUnityPlayerAudioData(void *data, int length);
Export(bool) GetIsPaused();
Export(void) SetPaused(bool paused);
Export(void) SeekFrameByPercent(float percent, void *data, int length);
Export(int) GetPlayerAudioSampleRate();
Export(int) GetPlayerAudioDurationOfSecondsNumber();
Export(int) GetPlayerVideoWidth();
Export(int) GetPlayerVideoHeight();
Export(void) CloseNewPlayer();

// audiowave
Export(int) InitAudioWaveContext(char *srcFilePath);
Export(int) GetAudioMetaDataLength(int id);
Export(double) GetAudioSecondsOfDuration(int id);
Export(int) GetAudioSampleRate(int id);
Export(int) GetAudioBitRate(int id);
Export(void) GetWaveMetaData(void *result, int id);
Export(void) DestroyAudioWaveContext(int id);

// videoseekcomponent
Export(int) InitSeekComponent(char *srcFilePath);
Export(int) GetSeekVideoWidth(int id);
Export(int) GetSeekVideoHeight(int id);
Export(double) GetSeekDurationSeconds(int id);
Export(void) SeekVideoFrameByPercent(float percent, void *data, int length, int id);
Export(void) DestroySeekComponent(int id);