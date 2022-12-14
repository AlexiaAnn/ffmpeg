
#include "ffmpeg.h"
//宏定义
#define EXPORTBUILD
void _DLLExport Mp4ToMp3(char *srcFileName, char *dstFileName)
{
    av_log_info("mp4 to mp3 start");
    std::thread th([srcFileName, dstFileName]()
                   {AudioFileContext context(AV_CODEC_ID_MP3);context.ExtractAudioToFile(srcFileName, dstFileName); });
    th.join();
    av_log_info("mp4 to mp3 end");
}
void _DLLExport InitCSharpDelegate(void (*Log)(char *message, int iSize), void (*LogError)(char *message, int iSize))
{
    Debug::LogFunPtr = Log;
    Debug::LogErrorFunPtr = LogError;
    av_log_info("Cpp Message:Log has initialized");
    av_log_info("ffmpeg version:%d,id:%d", 10, 20);
}
bool _DLLExport RecordMP4Start(const char *dstFilePath, int width, int height, int fps)
{
    // av_log_set_callback(UnityLogCallbackFunc);
    std::thread createVideoCtxThread([width, height, fps]()
                                     { videoContext = new VideoFileContext(AV_PIX_FMT_RGBA, AV_CODEC_ID_H264, fps, width, height); });
    createVideoCtxThread.join();
    if (videoContext == nullptr || videoContext->GetRet() < 0)
        return false;
    videoContext->WriteVideoPreparition(dstFilePath);
    return true;
}
void _DLLExport WriteMP4Frame(void *dataPtr, int length)
{
    videoContext->WriteVideoToFile(dataPtr, length);
}
void _DLLExport RecordMP4End()
{
    videoContext->WriteVideoTailer();
    delete videoContext;
    videoContext = nullptr;
}
bool _DLLExport RecordAVStartWithLogToUnity(const char *dstFilePath, int sampleRate, int channelCount, int width, int height, int fps)
{
    av_log_set_callback(UnityLogCallbackFunc);
    return RecordAVStart(dstFilePath, sampleRate, channelCount, width, height, fps);
}
bool _DLLExport RecordAVStartWithLogToTxt(const char *dstFilePath, int sampleRate, int channelCount, int width, int height, int fps)
{
    av_log_set_callback(LogCallbackTotxt);
    return RecordAVStart(dstFilePath, sampleRate, channelCount, width, height, fps);
}
bool _DLLExport RecordAVStart(const char *dstFilePath, int sampleRate, int channelCount, int width, int height, int fps)
{
    // av_log_set_callback(LogCallbackTotxt);
    std::thread createVideoCtxThread([dstFilePath, sampleRate, channelCount, width, height, fps]()
                                     {
                                         AVChannelLayout layout;
                                         if (channelCount == 1) {
                                             layout = { AV_CHANNEL_ORDER_NATIVE, (1), AV_CH_LAYOUT_MONO };
                                         }
                                         else if (channelCount == 2) {
                                             layout = { AV_CHANNEL_ORDER_NATIVE, (2), AV_CH_LAYOUT_STEREO };
                                         }
                                         else {
                                             av_log_error("the channel count isn`t be supported");
                                             return;
                                         }
                                         try
                                         {
                                             vaContext = new AVContext(dstFilePath, sampleRate, AV_SAMPLE_FMT_FLT, layout, AV_PIX_FMT_RGBA, fps, width, height);
                                         }
                                         catch (const std::exception& e)
                                         {
                                             av_log_error("vacontext initialize failed");
                                         } });
    createVideoCtxThread.join();
    if (vaContext == nullptr)
        return false;
    if (vaContext->GetRet() < 0)
    {
        delete vaContext;
        vaContext = nullptr;
        return false;
    }
    try
    {
        vaContext->WriteAVPreparition(dstFilePath);
        av_log_info("preparation is successful");
        return true;
    }
    catch (const std::exception &e)
    {
        av_log_error("preparation is failed,maybe:io exception,%s", e.what());
        return false;
    }
    return true;
}

void _DLLExport WriteVideoFrame(void *dataPtr)
{
    if (vaContext == nullptr)
    {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        vaContext->Flip((unsigned char *)dataPtr);
        vaContext->WriteVideoToFile(dataPtr, 0);
        av_log_info("write a video frame success");
    }
    catch (const std::exception &e)
    {
        av_log_error("write a video frame failed", e.what());
    }
}

void _DLLExport WriteAudioFrame(void *dataPtr, int length)
{
    if (vaContext == nullptr)
    {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        vaContext->WriteAudioToFile(dataPtr, length);
        av_log_info("write a audio frame success");
    }
    catch (const std::exception &e)
    {
        av_log_error("write a audio frame failed,%s", e.what());
    }
}

void _DLLExport FlushVideoBuffer()
{
    if (vaContext == nullptr)
    {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        vaContext->FlushEnVideoCodecBuffer();
        av_log_info("flush videoCodecBuffer success");
    }
    catch (const std::exception &e)
    {
        av_log_error("flush videoCodecBuffer failed,%s", e.what());
    }
}

void _DLLExport RecordAVEnd()
{
    if (vaContext == nullptr)
    {
        av_log_error("the vacontext is nullptr,can`t end record");
        return;
    }
    try
    {
        vaContext->FlushEnAudioCodecBuffer();
        vaContext->WriteAVTailer();
        delete vaContext;
    }
    catch (const std::exception &e)
    {
        av_log_error("record end failed,%s", e.what());
    }
    vaContext = nullptr;
    av_log_info("context delete,record end");
}

bool _DLLExport InitPlayer(char *srcFilePath)
{
    std::thread playerThread([srcFilePath]()
                             { player = new AVPlayer(srcFilePath); });
    playerThread.join();
    if (player == nullptr)
        return false;
    /*if (player->GetRet() < 0) {
        delete player;
        player = nullptr;
        return false;
    }*/
    return true;
}

void *_DLLExport GetRGBAByTargetPercent(float percent)
{
    const AVFrame *frame = player->GetFrameAtTargetPercent(percent);
    // player->Flip(frame->data[0]);
    return frame->data[0];
}

void *_DLLExport GetNextRGBAData()
{
    return player->GetNextVideoData();
}

void *_DLLExport GetNextPCMData()
{
    return player->GetNextAudioData();
}

bool _DLLExport GetIsVideoEnd()
{
    return player->GetIsVideoEnd();
}

bool _DLLExport GetIsAudioEnd()
{
    return player->GetIsAudioEnd();
}

int _DLLExport GetAVFileFrameCount()
{
    return player->GetAVFrameCount();
}

int _DLLExport GetVideoWidth()
{
    return player->GetVideoWidth();
}

int _DLLExport GetVideoHeight()
{
    return player->GetVideoHeight();
}

void _DLLExport ClosePlayer()
{
    if (player == nullptr)
        return;
    delete player;
    player = nullptr;
}

bool _DLLExport InitNewPlayer(char *srcFilePath)
{
    std::thread playerThread([srcFilePath]()
                             { newPlayer = new NewPlayer(srcFilePath); });
    playerThread.join();
    if (newPlayer == nullptr)
        return false;
    /*if (player->GetRet() < 0) {
        delete player;
        player = nullptr;
        return false;
    }*/
    newPlayer->InitData();
    return true;
}

bool _DLLExport SetUnityPlayerVideoData(void *data)
{
    return newPlayer->CopyVideoDataToUnity(data);
}

bool _DLLExport SetUnityPlayerAudioData(void *data, int length)
{
    av_log_info("SetUnityPlayerAudioData is be used\n");
    return newPlayer->CopyAudioDataToUnity(data, length);
}

bool _DLLExport GetIsPaused()
{

    return newPlayer->IsPaused();
}

void _DLLExport SetPaused(bool paused)
{
    newPlayer->SetPaused(paused);
}

void _DLLExport SeekFrameByPercent(float percent, void *data, int length)
{
    newPlayer->SeekFrameByPercent(percent, data, length);
}

int _DLLExport GetPlayerAudioSampleRate()
{
    return newPlayer->GetAudioSampleRate();
}

int _DLLExport GetPlayerAudioDurationOfSecondsNumber()
{
    return newPlayer->GetAudioSeconds();
}

int _DLLExport GetPlayerVideoWidth()
{
    return newPlayer->GetVideoWidth();
}

int _DLLExport GetPlayerVideoHeight()
{
    return newPlayer->GetVideoHeight();
}

void _DLLExport CloseNewPlayer()
{
    if (newPlayer == nullptr)
        return;
    delete newPlayer;
    newPlayer = nullptr;
}

int _DLLExport InitAudioWaveContext(char *srcFilePath)
{
    av_log_info("initing audio wave context\n");
    AudioWave *audioWave = nullptr;
    std::thread waveThread([&audioWave, srcFilePath]()
                           { audioWave = new AudioWave(srcFilePath); });
    waveThread.join();
    if (audioWave->GetRet() < 0)
    {
        delete audioWave;
        audioWave = nullptr;
        av_log_info("initing audio wave failed\n");
        return -1;
    }
    waveVectorMutex.lock();
    audioWaves.push_back(audioWave);
    int result = audioWaves.size() - 1;
    waveVectorMutex.unlock();
    av_log_info("initing audio wave success\n");
    return result;
}

int _DLLExport GetAudioMetaDataLength(int id)
{
    ID_CHECK_RETUREZERO
    waveVectorMutex.lock();
    int result = audioWaves[id]->GetMetaDataLength();
    waveVectorMutex.unlock();
    return result;
}

int _DLLExport GetAudioSampleRate(int id)
{
    ID_CHECK_RETUREZERO
    waveVectorMutex.lock();
    int result = audioWaves[id]->GetAudioSampleRate();
    waveVectorMutex.unlock();
    return result;
}
int _DLLExport GetAudioBitRate(int id)
{
    ID_CHECK_RETUREZERO
    waveVectorMutex.lock();
    int result = audioWaves[id]->GetAudioBitRate();
    waveVectorMutex.unlock();
    return result;
}

void _DLLExport GetWaveMetaData(void *result, int id)
{
    ID_CHECK_NORETURE
    waveVectorMutex.lock();
    AudioWave *point = audioWaves[id];
    waveVectorMutex.unlock();
    point->GetMetaData((float *)result);
}

void _DLLExport DestroyAudioWaveContext(int id)
{
    ID_CHECK_NORETURE
    waveVectorMutex.lock();
    delete audioWaves[id];
    audioWaves[id] = nullptr;
    waveVectorMutex.unlock();
}

double _DLLExport GetAudioSecondsOfDuration(int id)
{
    ID_CHECK_RETUREZERO
    waveVectorMutex.lock();
    AudioWave *point = audioWaves[id];
    waveVectorMutex.unlock();
    return point->GetSecondsOfDuration();
}

int _DLLExport InitSeekComponent(char *srcFilePath)
{
    av_log_info("init seek component start\n");
    SeekComponent *seekComponent = nullptr;
    std::thread seekThread([&seekComponent, srcFilePath]()
                           { seekComponent = new SeekComponent(srcFilePath); });
    seekThread.join();
    if (seekComponent->GetRet() < 0)
    {
        delete seekComponent;
        seekComponent = nullptr;
        av_log_error("init seek component failed\n");
        return -1;
    }
    seekVectorMutex.lock();
    seekCpts.push_back(seekComponent);
    int result = seekCpts.size() - 1;
    seekVectorMutex.unlock();
    av_log_info("init seek component success\n");
    return result;
}

int _DLLExport GetSeekVideoWidth(int id)
{
    ID_CHECK_RETUREZERO
    seekVectorMutex.lock();
    SeekComponent *point = seekCpts[id];
    seekVectorMutex.unlock();
    return point->GetVideoWidth();
}

int _DLLExport GetSeekVideoHeight(int id)
{
    ID_CHECK_RETUREZERO
    seekVectorMutex.lock();
    SeekComponent *point = seekCpts[id];
    seekVectorMutex.unlock();
    return point->GetVideoHeight();
}
double _DLLExport GetSeekDurationSeconds(int id)
{
    ID_CHECK_RETUREZERO
    seekVectorMutex.lock();
    SeekComponent *point = seekCpts[id];
    seekVectorMutex.unlock();
    return point->GetDuration();
}
void _DLLExport SeekVideoFrameByPercent(float percent, void *data, int length, int id)
{
    ID_CHECK_NORETURE
    av_log_info("seek video frame start\n");
    seekVectorMutex.lock();
    SeekComponent *point = seekCpts[id];
    seekVectorMutex.unlock();
    point->GetFrameDataByPercent(percent, data, length);
    av_log_info("seek video frame end\n");
}
void _DLLExport DestroySeekComponent(int id)
{
    ID_CHECK_NORETURE
    seekVectorMutex.lock();
    delete seekCpts[id];
    seekCpts[id] = nullptr;
    seekVectorMutex.unlock();
}
