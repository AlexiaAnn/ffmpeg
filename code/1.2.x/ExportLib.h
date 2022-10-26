#pragma once
#include "SeekVideo.h"
#include "RecordGif.h"
#include "RecordMp4.h"
#include "ExtractAudioFromMp4.h"
#include "AudioWaveA.h"
#include "ComponentManager.h"
#define Export(type)  extern "C" __declspec(dllexport) type __stdcall
#define ID_CHECK_RETUREZERO if (id < 0) {av_log_info("c++ dll id is incorrect,dll can`t find target context point\n");return 0;}
#define ID_CHECK_NORETURE if (id < 0) {av_log_info("c++ dll id is incorrect,dll can`t find target context point\n");}
RecordMp4* vaContext = nullptr;
RecordGif* recordGif=nullptr;



Export(void) Mp4ToMp3(char* srcFileName, char* dstFileName);
Export(void) InitCSharpDelegate(void (*Log)(char* message, int iSize), void (*LogError)(char* message, int iSize),bool isNeedFFmpeglog);

//record mp4
Export(bool) RecordAVStart(const char* dstFilePath,int sampleRate,int channelCount, int width, int height, int fps, float bitRatePercent);
Export(void) WriteVideoFrame(void* dataPtr);
Export(void) WriteAudioFrame(void* dataPtr,int length);
Export(void) FlushVideoBuffer();
Export(void) RecordAVEnd();

//audiowave
ComponentManager<AudioWaveA> audioWaves;
Export(int) InitAudioWaveContext(char* srcFilePath);
Export(int) GetAudioMetaDataLength(int id);
Export(double) GetAudioSecondsOfDuration(int id);
Export(int) GetAudioSampleRate(int id);
Export(int) GetAudioBitRate(int id);
Export(void) GetWaveMetaData(void* result,int id);
Export(void) DestroyAudioWaveContext(int id);

//videoseekcomponent
ComponentManager<SeekVideo> seekCpts;
Export(int) InitSeekComponent(char* srcFilePath);
Export(int) GetSeekVideoWidth(int id);
Export(int) GetSeekVideoHeight(int id);
Export(double) GetSeekDurationSeconds(int id);
Export(void) SeekVideoFrameByPercent(float percent, void* data, int length,int id);
Export(void) DestroySeekComponent(int id);

//record gif
Export(bool) RecordGifStart(const char* dstFilePath, int width, int height, int fps,float bitratePercent);
Export(void) WriteGifFrame(void* dataPtr);
Export(void) FlushGifBuffer();
Export(void) RecordGifEnd();