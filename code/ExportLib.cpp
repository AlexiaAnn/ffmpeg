#include "ExportLib.h"
#include <thread>
__declspec(dllexport) void __stdcall Mp4ToMp3(char *srcFileName, char *dstFileName)
{

    av_log_info("mp4 to mp3 start");
    std::thread th([srcFileName, dstFileName]()
                   {ExtractAudioFromMp4 context(AV_CODEC_ID_MP3);context.ExtractAudioToFile(srcFileName, dstFileName); });
    th.join();
    av_log_info("mp4 to mp3 end");
}
__declspec(dllexport) void __stdcall InitCSharpDelegate(void (*Log)(char *message, int iSize), void (*LogError)(char* message, int iSize))
{
    av_log_set_callback(UnityLogCallbackFunc);
    Debug::LogFunPtr = Log;
    Debug::LogErrorFunPtr = LogError;
    av_log_info("Cpp Message:Log has initialized");
    av_log_info("ffmpeg version:%d,id:%d",10,20);
}
__declspec(dllexport) bool __stdcall RecordMP4Start(const char *dstFilePath, int width, int height, int fps)
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
__declspec(dllexport) void __stdcall WriteMP4Frame(void *dataPtr, int length)
{
    videoContext->WriteVideoToFile(dataPtr, length);
}
__declspec(dllexport) void __stdcall RecordMP4End()
{
    videoContext->WriteVideoTailer();
    delete videoContext;
    videoContext = nullptr;
}
__declspec(dllexport) bool __stdcall RecordAVStartWithLogToUnity(const char* dstFilePath, int sampleRate, int channelCount, int width, int height, int fps, float bitRatePercent) {
    av_log_set_callback(UnityLogCallbackFunc);
    return RecordAVStart(dstFilePath,sampleRate,channelCount,width,height,fps,bitRatePercent);
}
__declspec(dllexport) bool __stdcall RecordAVStartWithLogToTxt(const char* dstFilePath, int sampleRate, int channelCount, int width, int height, int fps, float bitRatePercent)
{
    av_log_set_callback(LogCallbackTotxt);
    return RecordAVStart(dstFilePath, sampleRate, channelCount, width, height, fps,bitRatePercent);
}
__declspec(dllexport) bool __stdcall RecordAVStart(const char *dstFilePath, int sampleRate, int channelCount, int width, int height, int fps,float bitRatePercent)
{
    //av_log_set_callback(LogCallbackTotxt);
    std::thread createVideoCtxThread([dstFilePath, sampleRate, channelCount, width, height, fps,bitRatePercent]()
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
                                             vaContext = new AVContext(dstFilePath, sampleRate, AV_SAMPLE_FMT_FLT, layout, AV_PIX_FMT_RGBA, fps, bitRatePercent,width, height);
                                         }
                                         catch (const std::exception& e)
                                         {
                                             av_log_error("vacontext initialize failed");
                                         } });
    createVideoCtxThread.join();
    if (vaContext == nullptr)
        return false;
    if (vaContext->GetRet() < 0) {
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
    catch (const std::exception& e)
    {
        av_log_error("preparation is failed,maybe:io exception,%s",e.what());
        return false;
    }
    return true;
}

__declspec(dllexport) void __stdcall WriteVideoFrame(void* dataPtr)
{
    if (vaContext == nullptr) {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        vaContext->Flip((unsigned char*)dataPtr);
        vaContext->WriteVideoToFile(dataPtr,0);
        av_log_info("write a video frame success");
    }
    catch (const std::exception& e)
    {
        av_log_error("write a video frame failed",e.what());
    }
}

__declspec(dllexport) void __stdcall WriteAudioFrame(void* dataPtr, int length)
{
    if (vaContext == nullptr) {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        vaContext->WriteAudioToFile(dataPtr, length);
        av_log_info("write a audio frame success");
    }
    catch (const std::exception& e)
    {
        av_log_error("write a audio frame failed,%s",e.what());
    }
    
}

__declspec(dllexport) void __stdcall FlushVideoBuffer()
{
    if (vaContext == nullptr) {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        vaContext->FlushEnVideoCodecBuffer();
        av_log_info("flush videoCodecBuffer success");
    }
    catch (const std::exception& e)
    {
        av_log_error("flush videoCodecBuffer failed,%s", e.what());
    }
    
}

__declspec(dllexport) void __stdcall RecordAVEnd()
{
    if (vaContext == nullptr) {
        av_log_error("the vacontext is nullptr,can`t end record");
        return;
    }
    try
    {
        vaContext->FlushEnAudioCodecBuffer();
        vaContext->WriteAVTailer();
        delete vaContext;
    }
    catch (const std::exception& e)
    {
        av_log_error("record end failed,%s",e.what());
    }
    vaContext = nullptr;
    av_log_info("context delete,record end");
}

__declspec(dllexport) bool __stdcall InitPlayer(char* srcFilePath)
{
    std::thread playerThread([srcFilePath]() {
        player = new AVPlayer(srcFilePath);
        });
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

__declspec(dllexport) void* __stdcall GetRGBAByTargetPercent(float percent)
{
    const AVFrame* frame = player->GetFrameAtTargetPercent(percent);
    //player->Flip(frame->data[0]);
    return frame->data[0];
}

__declspec(dllexport) void* __stdcall GetNextRGBAData()
{
    return player->GetNextVideoData();
}

__declspec(dllexport) void* __stdcall GetNextPCMData()
{
    return player->GetNextAudioData();
}

bool __stdcall GetIsVideoEnd()
{
    return player->GetIsVideoEnd();
}

bool __stdcall GetIsAudioEnd()
{
    return player->GetIsAudioEnd();
}

__declspec(dllexport) int __stdcall GetAVFileFrameCount()
{
    return player->GetAVFrameCount();
}

__declspec(dllexport) int __stdcall GetVideoWidth()
{
    return player->GetVideoWidth();
}

__declspec(dllexport) int __stdcall GetVideoHeight()
{
    return player->GetVideoHeight();
}

__declspec(dllexport) void __stdcall ClosePlayer()
{
    if (player == nullptr) return;
    delete player;
    player = nullptr;
}


__declspec(dllexport) bool __stdcall InitNewPlayer(char* srcFilePath)
{
    std::thread playerThread([srcFilePath]() {
        newPlayer = new NewPlayer(srcFilePath);
        });
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

__declspec(dllexport) bool __stdcall SetUnityPlayerVideoData(void* data)
{
    return newPlayer->CopyVideoDataToUnity(data);
}

__declspec(dllexport) bool __stdcall SetUnityPlayerAudioData(void* data, int length)
{
    av_log_info("SetUnityPlayerAudioData is be used\n");
    return newPlayer->CopyAudioDataToUnity(data, length);
}

__declspec(dllexport) bool __stdcall GetIsPaused()
{

    return newPlayer->IsPaused();
}

__declspec(dllexport) void __stdcall SetPaused(bool paused)
{
    newPlayer->SetPaused(paused);
}

__declspec(dllexport) void __stdcall SeekFrameByPercent(float percent, void* data,int length)
{
    newPlayer->SeekFrameByPercent(percent, data,length);
}

int __stdcall GetPlayerAudioSampleRate()
{
    return newPlayer->GetAudioSampleRate();
}

int __stdcall GetPlayerAudioDurationOfSecondsNumber()
{
    return newPlayer->GetAudioSeconds();
}

__declspec(dllexport) int __stdcall GetPlayerVideoWidth()
{
    return newPlayer->GetVideoWidth();
}

__declspec(dllexport) int __stdcall GetPlayerVideoHeight()
{
    return newPlayer->GetVideoHeight();
}

__declspec(dllexport) void __stdcall CloseNewPlayer()
{
    if (newPlayer == nullptr) return;
    delete newPlayer;
    newPlayer = nullptr;
}

__declspec(dllexport) int __stdcall InitAudioWaveContext(char* srcFilePath)
{
    av_log_info("initing audio wave context\n");
    AudioWave* audioWave = nullptr;
    std::thread waveThread([&audioWave,srcFilePath]() {
        audioWave = new AudioWave(srcFilePath);
        });
    waveThread.join();
    if (audioWave->GetRet() < 0) {
        delete audioWave;
        audioWave = nullptr;
        av_log_info("initing audio wave failed\n");
        return -1;
    }
    waveVectorMutex.lock();
    audioWaves.push_back(audioWave);
    int result = audioWaves.size()-1;
    waveVectorMutex.unlock();
    av_log_info("initing audio wave success\n");
    return result;
}

__declspec(dllexport) int __stdcall GetAudioMetaDataLength(int id)
{
    ID_CHECK_RETUREZERO
    waveVectorMutex.lock();
    int result = audioWaves[id]->GetMetaDataLength();
    waveVectorMutex.unlock();
    return result;
}

__declspec(dllexport) int __stdcall GetAudioSampleRate(int id)
{
    ID_CHECK_RETUREZERO
    waveVectorMutex.lock();
    int result = audioWaves[id]->GetAudioSampleRate();
    waveVectorMutex.unlock();
    return result;
}
__declspec(dllexport) int __stdcall GetAudioBitRate(int id)
{
    ID_CHECK_RETUREZERO
    waveVectorMutex.lock();
    int result = audioWaves[id]->GetAudioBitRate();
    waveVectorMutex.unlock();
    return result;
}

__declspec(dllexport) void __stdcall GetWaveMetaData(void* result,int id)
{
    ID_CHECK_NORETURE
    waveVectorMutex.lock();
    AudioWave* point = audioWaves[id];
    waveVectorMutex.unlock();
    point->GetMetaData((float*)result);
}

__declspec(dllexport) void __stdcall DestroyAudioWaveContext(int id)
{
    ID_CHECK_NORETURE
    waveVectorMutex.lock();
    delete audioWaves[id];
    audioWaves[id] = nullptr;
    waveVectorMutex.unlock();
}

__declspec(dllexport) double __stdcall GetAudioSecondsOfDuration(int id)
{
    ID_CHECK_RETUREZERO
    waveVectorMutex.lock();
    AudioWave* point = audioWaves[id];
    waveVectorMutex.unlock();
    return point->GetSecondsOfDuration();
}

__declspec(dllexport) int __stdcall InitSeekComponent(char* srcFilePath)
{
    av_log_info("init seek component start\n");
    SeekComponent* seekComponent = nullptr;
    std::thread seekThread([&seekComponent,srcFilePath]() {
        seekComponent = new SeekComponent(srcFilePath);
        });
    seekThread.join();
    if (seekComponent->GetRet() < 0) {
        delete seekComponent;
        seekComponent = nullptr;
        av_log_error("init seek component failed\n");
        return -1;
    }
    seekVectorMutex.lock();
    seekCpts.push_back(seekComponent);
    int result = seekCpts.size()-1;
    seekVectorMutex.unlock();
    av_log_info("init seek component success\n");
    return result;
}

__declspec(dllexport) int __stdcall GetSeekVideoWidth(int id)
{
    ID_CHECK_RETUREZERO
    seekVectorMutex.lock();
    SeekComponent* point = seekCpts[id];
    seekVectorMutex.unlock();
    return point->GetVideoWidth();
}

__declspec(dllexport) int __stdcall GetSeekVideoHeight(int id)
{
    ID_CHECK_RETUREZERO
    seekVectorMutex.lock();
    SeekComponent* point = seekCpts[id];
    seekVectorMutex.unlock();
    return point->GetVideoHeight();
}
__declspec(dllexport) double __stdcall GetSeekDurationSeconds(int id)
{
    ID_CHECK_RETUREZERO
    seekVectorMutex.lock();
    SeekComponent* point = seekCpts[id];
    seekVectorMutex.unlock();
    return point->GetDuration();
}
__declspec(dllexport) void __stdcall SeekVideoFrameByPercent(float percent, void* data, int length,int id)
{
    ID_CHECK_NORETURE
    av_log_info("seek video frame start\n");
    seekVectorMutex.lock();
    SeekComponent* point = seekCpts[id];
    seekVectorMutex.unlock();
    point->GetFrameDataByPercent(percent,data,length);
    av_log_info("seek video frame end\n");
}
__declspec(dllexport) void __stdcall DestroySeekComponent(int id)
{
    ID_CHECK_NORETURE
    seekVectorMutex.lock();
    delete seekCpts[id];
    seekCpts[id] = nullptr;
    seekVectorMutex.unlock();
}

__declspec(dllexport) bool __stdcall RecordGifStart(const char* dstFilePath, int width, int height, int fps,float bitRatePercent)
{
    std::thread createVideoCtxThread([dstFilePath, width, height, fps,bitRatePercent]()
        {
            try
            {
                recordGif = new RecordGif(dstFilePath, AV_PIX_FMT_RGBA, fps, bitRatePercent, width, height);
            }
            catch (const std::exception& e)
            {
                av_log_error("vacontext initialize failed");
            } });
    //程序未执行之后的代码，需要调试
    createVideoCtxThread.join();
    if (recordGif == nullptr)
        return false;
    if (recordGif->GetResult() < 0) {
        delete vaContext;
        recordGif = nullptr;
        return false;
    }
    try
    {
        recordGif->WriteGIFPreparition();
        av_log_info("preparation is successful");
        return true;
    }
    catch (const std::exception& e)
    {
        av_log_error("preparation is failed,maybe:io exception,%s", e.what());
        return false;
    }
    return true;
}

__declspec(dllexport) void __stdcall WriteGifFrame(void* dataPtr)
{
    if (recordGif == nullptr) {
        av_log_error("the recordGif is nullptr,write failed");
        return;
    }
    try
    {
        //recordGif->Flip((unsigned char*)dataPtr);
        recordGif->WriteVideoToFile(dataPtr, 0);
        av_log_info("write a video frame success");
    }
    catch (const std::exception& e)
    {
        av_log_error("write a video frame failed", e.what());
    }
}

__declspec(dllexport) void __stdcall FlushGifBuffer()
{
    if (recordGif == nullptr) {
        av_log_error("the recordGif is nullptr,write failed");
        return;
    }
    try
    {
        recordGif->FlushEnVideoCodecBuffer();
        av_log_info("flush videoCodecBuffer success");
    }
    catch (const std::exception& e)
    {
        av_log_error("flush videoCodecBuffer failed,%s", e.what());
    }
}

__declspec(dllexport) void __stdcall RecordGifEnd()
{
    if (recordGif == nullptr) {
        av_log_error("the recordGif is nullptr,can`t end record");
        return;
    }
    try
    {
        recordGif->WriteGIFTailer();
        delete recordGif;
    }
    catch (const std::exception& e)
    {
        av_log_error("record end failed,%s", e.what());
    }
    recordGif = nullptr;
    av_log_info("context delete,record end");
}


