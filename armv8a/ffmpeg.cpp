
#include "ffmpeg.h"
//宏定义
#define EXPORTBUILD
void _DLLExport Mp4ToMp3(char *srcFileName, char *dstFileName, LogFunc func)
{
    std::thread th([srcFileName, dstFileName]()
                   {AudioContext context(AV_CODEC_ID_MP3);
    context.ExtractAudioToFile(srcFileName, dstFileName); });
    th.join();
}
void _DLLExport InitCSharpDelegate(void (*Log)(char *message, int iSize))
{
    Debug::LogFunPtr = Log;
    av_log_info("Cpp Message:Log has initialized");
}
bool _DLLExport RecordMP4Start(const char *dstFilePath, int width, int height, int fps)
{
    // av_log_set_callback(UnityLogCallbackFunc);
    av_log_info("width:%d,height:%d", width, height);
    std::thread createVideoCtxThread([width, height, fps]()
                                     { videoContext = new VideoContext(AV_PIX_FMT_RGBA, AV_CODEC_ID_H264, fps, width, height); });

    createVideoCtxThread.join();
    if (videoContext == nullptr || videoContext->ret < 0)
        return false;
    videoContext->WriteFramePrepare(dstFilePath);
    return true;
}
void _DLLExport WriteMP4Frame(void *dataPtr, int length)
{
    videoContext->WriteFrame(dataPtr, length);
}
void _DLLExport RecordMP4End()
{
    videoContext->WriteFrameTailer();
    delete videoContext;
    videoContext = nullptr;
}
