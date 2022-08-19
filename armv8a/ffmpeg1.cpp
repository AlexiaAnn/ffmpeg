
#include "ffmpeg1.h"
//宏定义
#define EXPORTBUILD
void _DLLExport Mp4ToMp3(char *srcFileName, char *dstFileName, LogFunc func)
{

    av_log_set_callback(FFmpegLogCallbackFunc);
    std::thread th([srcFileName, dstFileName]()
                   { context = new MP42MP3Context(srcFileName, dstFileName); });
    th.join();
    context->Mp42Mp3();
}
void _DLLExport InitCSharpDelegate(void (*Log)(char *message, int iSize))
{
    Debug::LogFunPtr = Log;
    av_log_info("Cpp Message:Log has initialized");
}