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
__declspec(dllexport) void __stdcall InitCSharpDelegate(void (*Log)(char *message, int iSize), void (*LogError)(char* message, int iSize),bool isNeedFFmpegLog)
{
    if(isNeedFFmpegLog)av_log_set_callback(UnityLogCallbackFunc);
    Debug::LogFunPtr = Log;
    Debug::LogErrorFunPtr = LogError;
    av_log_info("Cpp Message:Log has initialized");
    av_log_info("ffmpeg version:%s,id:%s","5.10", "1.2.1");
}

__declspec(dllexport) bool __stdcall RecordAVStart(const char *dstFilePath, int sampleRate, int channelCount, int width, int height, int fps,float bitRatePercent)
{
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
                                             vaContext = new RecordMp4(dstFilePath, sampleRate, AV_SAMPLE_FMT_FLT, layout, AV_PIX_FMT_RGBA, fps, bitRatePercent,width, height);
                                         }
                                         catch (const std::exception& e)
                                         {
                                             av_log_error("vacontext initialize failed");
                                         } });
    createVideoCtxThread.join();
    if (vaContext == nullptr)
        return false;
    if (vaContext->GetResult() < 0) {
        delete vaContext;
        vaContext = nullptr;
        return false;
    }
    try
    {
        vaContext->WriteAVPreparition();
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
        //vaContext->Flip((unsigned char*)dataPtr);
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

__declspec(dllexport) int __stdcall InitAudioWaveContext(char* srcFilePath)
{
    av_log_info("initing audio wave context\n");
    AudioWaveA* audioWave = nullptr;
    std::thread waveThread([&audioWave,srcFilePath]() {
        audioWave = new AudioWaveA(srcFilePath);
        });
    waveThread.join();
    if (audioWave->GetResult() < 0) {
        delete audioWave;
        audioWave = nullptr;
        av_log_info("initing audio wave failed\n");
        return -1;
    }
    int result = audioWaves.PushPointerAndGetid(audioWave);
    av_log_info("initing audio wave success\n");
    return result;
}

__declspec(dllexport) int __stdcall GetAudioMetaDataLength(int id)
{
    ID_CHECK_RETUREZERO
    return audioWaves.GetPointerById(id)->GetMetaDataLength();
}

__declspec(dllexport) int __stdcall GetAudioSampleRate(int id)
{
    ID_CHECK_RETUREZERO
        return audioWaves.GetPointerById(id)->GetAudioSampleRate();
}
__declspec(dllexport) int __stdcall GetAudioBitRate(int id)
{
    ID_CHECK_RETUREZERO
        return audioWaves.GetPointerById(id)->GetAudioBitRate();
}

__declspec(dllexport) void __stdcall GetWaveMetaData(void* result,int id)
{
    ID_CHECK_NORETURE
    audioWaves.GetPointerById(id)->GetMetaData((float*)result);
}

__declspec(dllexport) void __stdcall DestroyAudioWaveContext(int id)
{
    ID_CHECK_NORETURE
        audioWaves.DeletePointerByid(id);
}

__declspec(dllexport) double __stdcall GetAudioSecondsOfDuration(int id)
{
    ID_CHECK_RETUREZERO
        audioWaves.GetPointerById(id)->GetSecondsOfDuration();
}

__declspec(dllexport) int __stdcall InitSeekComponent(char* srcFilePath)
{
    av_log_info("init seek component start\n");
    SeekVideo* seekComponent = nullptr;
    std::thread seekThread([&seekComponent,srcFilePath]() {
        seekComponent = new SeekVideo(srcFilePath);
        });
    seekThread.join();
    if (seekComponent->GetResult() < 0) {
        delete seekComponent;
        seekComponent = nullptr;
        av_log_error("init seek component failed\n");
        return -1;
    }
    int result = seekCpts.PushPointerAndGetid(seekComponent);
    av_log_info("init seek component success\n");
    return result;
}

__declspec(dllexport) int __stdcall GetSeekVideoWidth(int id)
{
    ID_CHECK_RETUREZERO
        return seekCpts.GetPointerById(id)->GetVideoWidth();
}

__declspec(dllexport) int __stdcall GetSeekVideoHeight(int id)
{
    ID_CHECK_RETUREZERO
        return seekCpts.GetPointerById(id)->GetVideoHeight();
}
__declspec(dllexport) double __stdcall GetSeekDurationSeconds(int id)
{
    ID_CHECK_RETUREZERO
        return seekCpts.GetPointerById(id)->GetDuration();
}
__declspec(dllexport) void __stdcall SeekVideoFrameByPercent(float percent, void* data, int length,int id)
{
    ID_CHECK_NORETURE
    av_log_info("seek video frame start\n");
    seekCpts.GetPointerById(id)->GetFrameDataByPercent(percent, data, length);
    av_log_info("seek video frame end\n");
}
__declspec(dllexport) void __stdcall DestroySeekComponent(int id)
{
    ID_CHECK_NORETURE
        seekCpts.DeletePointerByid(id);
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


