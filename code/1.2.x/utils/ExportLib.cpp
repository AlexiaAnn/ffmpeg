#include "ExportLib.h"
Declspec void StdDll Mp4ToMp3(char *srcFileName, char *dstFileName)
{

    av_log_info("mp4 to mp3 start");
    std::thread th([srcFileName, dstFileName]()
                   {ExtractAudio context(srcFileName,dstFileName);context.DoExtract(); });
    th.join();
    av_log_info("mp4 to mp3 end");
}
Declspec void StdDll InitCSharpDelegate(void (*Log)(char *message, int iSize), void (*LogError)(char *message, int iSize), bool isNeedFFmpegLog)
{
    if (isNeedFFmpegLog)
        av_log_set_callback(UnityLogCallbackFunc);
    Debug::LogFunPtr = Log;
    Debug::LogErrorFunPtr = LogError;
    av_log_info("Cpp Message:Log has initialized");
    av_log_info("ffmpeg version:%s,ffmpeg2 date:%s", "5.10", "2022.11.8");
}

Declspec bool StdDll RecordAVStart(const char *dstFilePath, int sampleRate, int channelCount,
                                   int width, int height, int fps, float bitRatePercent,
                                   int crfMin, int crfMax, int presetLevel)
{
    std::thread createVideoCtxThread([dstFilePath, sampleRate, channelCount, width, height, fps, bitRatePercent, crfMin, crfMax, presetLevel]()
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
                                             vaContext = new RecordMp4(dstFilePath, sampleRate, AV_SAMPLE_FMT_FLT, layout, AV_PIX_FMT_RGBA, fps, bitRatePercent,width, height,crfMin,crfMax, presetLevel);
                                         }
                                         catch (const std::exception& e)
                                         {
                                             av_log_error("vacontext initialize failed");
                                         } });
    createVideoCtxThread.join();
    if (vaContext == nullptr)
        return false;
    if (vaContext->GetResult() < 0)
    {
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
    catch (const std::exception &e)
    {
        av_log_error("preparation is failed,maybe:io exception,%s", e.what());
        return false;
    }
    return true;
}

Declspec void StdDll WriteVideoFrame(void *dataPtr)
{
    if (vaContext == nullptr)
    {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        iVideoFrameCount++;
        // vaContext->Flip((unsigned char*)dataPtr);
        videoFrameStart = clock();
        vaContext->WriteVideoToFile(dataPtr, 0);
        videoFrameEnd = clock();
        float duration = float(videoFrameEnd - videoFrameStart) / CLOCKS_PER_SEC;
        videoFrameAllTime += duration;
        // av_log_info("write a video frame duration:%f", duration);
        av_log_pframe("write a video frame success");
    }
    catch (const std::exception &e)
    {
        av_log_error("write a video frame failed:%s", e.what());
    }
}

Declspec void StdDll WriteAudioFrame(void *dataPtr, int length)
{
    if (vaContext == nullptr)
    {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        vaContext->WriteAudioToFile(dataPtr, length);
        av_log_pframe("write a audio frame success");
    }
    catch (const std::exception &e)
    {
        av_log_error("write a audio frame failed,%s", e.what());
    }
}

Declspec void StdDll FlushVideoBuffer()
{
    if (vaContext == nullptr)
    {
        av_log_error("the vacontext is nullptr,write failed");
        return;
    }
    try
    {
        videoFrameStart = clock();
        vaContext->FlushEnVideoCodecBuffer();
        videoFrameEnd = clock();
        videoFrameAllTime += float(videoFrameEnd - videoFrameStart) / CLOCKS_PER_SEC;
        av_log_info("flush videoCodecBuffer success");
        av_log_info("video frame:[count:%d],[alltime:%f],[alltime avg:%f]",
                    iVideoFrameCount, videoFrameAllTime, videoFrameAllTime / iVideoFrameCount);
        iVideoFrameCount = 0;
        videoFrameAllTime = 0;
    }
    catch (const std::exception &e)
    {
        av_log_error("flush videoCodecBuffer failed,%s", e.what());
    }
}

Declspec void StdDll RecordAVEnd()
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

Declspec int StdDll InitAudioWaveContext(char *srcFilePath)
{
    av_log_info("initing audio wave context\n");
    AudioWaveA *audioWave = nullptr;
    std::thread waveThread([&audioWave, srcFilePath]()
                           { audioWave = new AudioWaveA(srcFilePath); });
    waveThread.join();
    if (audioWave->GetResult() < 0)
    {
        delete audioWave;
        audioWave = nullptr;
        av_log_error("initing audio wave failed\n");
        return -1;
    }
    int result = audioWaves.PushPointerAndGetid(audioWave);
    av_log_info("initing audio wave success\n");
    return result;
}

Declspec int StdDll GetAudioMetaDataLength(int id)
{
    ID_CHECK_RETUREZERO
    return audioWaves.GetPointerById(id)->GetMetaDataLength();
}

Declspec int StdDll GetAudioSampleRate(int id)
{
    ID_CHECK_RETUREZERO
    return audioWaves.GetPointerById(id)->GetAudioSampleRate();
}
Declspec int StdDll GetAudioBitRate(int id)
{
    ID_CHECK_RETUREZERO
    return audioWaves.GetPointerById(id)->GetAudioBitRate();
}

Declspec void StdDll GetWaveMetaData(void *result, int id)
{
    ID_CHECK_NORETURE
    audioWaves.GetPointerById(id)->GetMetaData((float *)result);
}

Declspec void StdDll DestroyAudioWaveContext(int id)
{
    ID_CHECK_NORETURE
    audioWaves.DeletePointerByid(id);
}

Declspec double StdDll GetAudioSecondsOfDuration(int id)
{
    ID_CHECK_RETUREZERO
    return audioWaves.GetPointerById(id)->GetSecondsOfDuration();
}

Declspec int StdDll InitSeekComponent(char *srcFilePath)
{
    av_log_info("init seek component start\n");
    SeekVideo *seekComponent = nullptr;
    std::thread seekThread([&seekComponent, srcFilePath]()
                           { seekComponent = new SeekVideo(srcFilePath); });
    seekThread.join();
    if (seekComponent->GetResult() < 0)
    {
        delete seekComponent;
        seekComponent = nullptr;
        av_log_error("init seek component failed\n");
        return -1;
    }
    int result = seekCpts.PushPointerAndGetid(seekComponent);
    av_log_info("init seek component success\n");
    return result;
}

Declspec int StdDll GetSeekVideoWidth(int id)
{
    ID_CHECK_RETUREZERO
    return seekCpts.GetPointerById(id)->GetVideoWidth();
}

Declspec int StdDll GetSeekVideoHeight(int id)
{
    ID_CHECK_RETUREZERO
    return seekCpts.GetPointerById(id)->GetVideoHeight();
}
Declspec double StdDll GetSeekDurationSeconds(int id)
{
    ID_CHECK_RETUREZERO
    return seekCpts.GetPointerById(id)->GetDuration();
}
Declspec void StdDll SeekVideoFrameByPercent(float percent, void *data, int length, int id)
{
    ID_CHECK_NORETURE
    av_log_pframe("seek video frame start");
    seekCpts.GetPointerById(id)->GetFrameDataByPercent(percent, data, length);
    av_log_pframe("seek video frame end");
}
Declspec void StdDll DestroySeekComponent(int id)
{
    ID_CHECK_NORETURE
    seekCpts.DeletePointerByid(id);
}

Declspec bool StdDll RecordGifStart(const char *dstFilePath, int width, int height, int fps, float bitRatePercent, int presetLevel)
{
    std::thread createVideoCtxThread([dstFilePath, width, height, fps, bitRatePercent, presetLevel]()
                                     {
            try
            {
                recordGif = new RecordGif(dstFilePath, AV_PIX_FMT_RGBA, fps, bitRatePercent, width, height,presetLevel);
            }
            catch (const std::exception& e)
            {
                av_log_error("vacontext initialize failed");
            } });
    //程序未执行之后的代码，需要调试
    createVideoCtxThread.join();
    if (recordGif == nullptr)
        return false;
    if (recordGif->GetResult() < 0)
    {
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
    catch (const std::exception &e)
    {
        av_log_error("preparation is failed,maybe:io exception,%s", e.what());
        return false;
    }
    return true;
}

Declspec void StdDll WriteGifFrame(void *dataPtr)
{
    if (recordGif == nullptr)
    {
        av_log_error("the recordGif is nullptr,write failed");
        return;
    }
    try
    {
        // recordGif->Flip((unsigned char*)dataPtr);
        recordGif->WriteVideoToFile(dataPtr, 0);
        av_log_pframe("write a video frame success");
    }
    catch (const std::exception &e)
    {
        av_log_error("write a video frame failed:%s", e.what());
    }
}

Declspec void StdDll FlushGifBuffer()
{
    if (recordGif == nullptr)
    {
        av_log_error("the recordGif is nullptr,write failed");
        return;
    }
    try
    {
        recordGif->FlushEnVideoCodecBuffer();
        av_log_info("flush videoCodecBuffer success");
    }
    catch (const std::exception &e)
    {
        av_log_error("flush videoCodecBuffer failed,%s", e.what());
    }
}

Declspec void StdDll RecordGifEnd()
{
    if (recordGif == nullptr)
    {
        av_log_error("the recordGif is nullptr,can`t end record");
        return;
    }
    try
    {
        recordGif->WriteGIFTailer();
        delete recordGif;
    }
    catch (const std::exception &e)
    {
        av_log_error("record end failed,%s", e.what());
    }
    recordGif = nullptr;
    av_log_info("context delete,record end");
}
