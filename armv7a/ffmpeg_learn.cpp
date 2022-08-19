
#include "ffmpeg_learn.h"
//宏定义
#define EXPORTBUILD
void _DLLExport Mp4ToMp3(char *srcFileName, char *dstFileName, char *logFileName)
{
    MP42MP3Context context(srcFileName, dstFileName);
    context.Mp42Mp3();
}
//相加
int _DLLExport Add(int x, int y)
{
    return x + y;
}

//取较大的值
int _DLLExport Max(int x, int y)
{
    return (x >= y) ? x : y;
}