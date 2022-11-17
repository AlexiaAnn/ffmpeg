#pragma once
#ifdef WINDOWS
#include "../components/read/SeekVideo.h"
#include "../components/record/RecordGif.h"
#include "../components/record/RecordMp4.h"
#include "../components/record/RecordMp4Thread.h"
#include "../components/extract/ExtractAudio.h"
#include "../components/read/AudioWaveA.h"
#include "../components/ComponentManager.h"
#include "../components/read/ReadHeif.h"
#endif // WINDOWS

#ifdef ANDROID
#include "components/read/SeekVideo.h"
#include "components/record/RecordGif.h"
#include "components/record/RecordMp4.h"
#include "components/record/RecordMp4Thread.h"
#include "components/extract/ExtractAudio.h"
#include "components/read/AudioWaveA.h"
#include "components/ComponentManager.h"
#include "components/read/ReadHeif.h"
#endif // ANDROID



#ifdef WINDOWS
#define Export(type) extern "C" __declspec(dllexport) type __stdcall
#define Declspec __declspec(dllexport)
#define StdDll __stdcall
#endif
#ifdef ANDROID
#define Export(type) extern "C" type __attribute__((visibility("default")))
#define Declspec
#define StdDll __attribute__((visibility("default")))
#endif
#define ID_CHECK_RETUREZERO                                                           \
	if (id < 0)                                                                       \
	{                                                                                 \
		av_log_info("c++ dll id is incorrect,dll can`t find target context point\n"); \
		return 0;                                                                     \
	}
#define ID_CHECK_NORETURE                                                             \
	if (id < 0)                                                                       \
	{                                                                                 \
		av_log_info("c++ dll id is incorrect,dll can`t find target context point\n"); \
		return;                                                                       \
	}
RecordMp4 *vaContext = nullptr;
RecordMp4Thread* recordMp4ThreadContext = nullptr;
RecordGif *recordGif = nullptr;
ReadHeif* heifPointer = nullptr;

clock_t videoFrameStart, videoFrameEnd;
float videoFrameAllTime = 0;
int iVideoFrameCount = 0;

// about ffmpeg

Export(void) Mp4ToMp3(char *srcFileName, char *dstFileName);
Export(void) InitCSharpDelegate(void (*Log)(char *message, int iSize), void (*LogError)(char *message, int iSize), bool isNeedFFmpeglog);

// record mp4
Export(bool) RecordAVStart(const char *dstFilePath, int sampleRate, int channelCount,
						   int width, int height, int fps, float bitRatePercent,
						   int crfMin, int crfMax, int presetLevel);
Export(void) WriteVideoFrame(void *dataPtr);
Export(void) WriteAudioFrame(void *dataPtr, int length);
Export(void) FlushVideoBuffer();
Export(void) RecordAVEnd();

//record mp4 thread
Export(bool) RecordAVStartThread(const char* dstFilePath, int sampleRate, int channelCount,
	int width, int height, int fps, float bitRatePercent,
	int crfMin, int crfMax, int presetLevel);
Export(void) WriteVideoFrameThread(void* dataPtr,int id);
Export(void) WriteAudioFrameThread(void* dataPtr, int length);
Export(void) FlushVideoBufferThread();
Export(void) FlushAudioBufferThread();
Export(void) RecordAVEndThread();
Export(void) DestroyRecordPointerThread();

// audiowave
ComponentManager<AudioWaveA> audioWaves;
Export(int) InitAudioWaveContext(char *srcFilePath);
Export(int) GetAudioMetaDataLength(int id);
Export(double) GetAudioSecondsOfDuration(int id);
Export(int) GetAudioSampleRate(int id);
Export(int) GetAudioBitRate(int id);
Export(void) GetWaveMetaData(void *result, int id);
Export(void) DestroyAudioWaveContext(int id);

// videoseekcomponent
ComponentManager<SeekVideo> seekCpts;
Export(int) InitSeekComponent(char *srcFilePath);
Export(int) GetSeekVideoWidth(int id);
Export(int) GetSeekVideoHeight(int id);
Export(double) GetSeekDurationSeconds(int id);
Export(void) SeekVideoFrameByPercent(float percent, void *data, int length, int id);
Export(void) DestroySeekComponent(int id);

// record gif
Export(bool) RecordGifStart(const char *dstFilePath, int width, int height, int fps, float bitratePercent, int presetLevel);
Export(void) WriteGifFrame(void *dataPtr);
Export(void) FlushGifBuffer();
Export(void) RecordGifEnd();

//read heif image
ComponentManager<ReadHeif> heifCpts;
Export(int) HeifStart(const char* srcPath);
Export(int) HeifImageWidth(int id);
Export(int) HeifImageHeight(int id);
Export(void) HeifToRgb(void* data,int id);
Export(void) DestroyHeifPointer(int id);