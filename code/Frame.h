#pragma once
#include "IncludeFFmpeg.h"
class Frame
{
public:
    AVFrame *frame;
    AVSubtitle sub;  // 字幕
    int serial;      // 播放序列
    double pts;      // 当前帧指向的pts 时间戳，单位为秒       /* presentation timestamp for the frame */
    double duration; // 当前帧所持续的时间    /* estimated duration of the frame */
    int64_t pos;     // 该帧在文件中的字节位置    /* byte position of the frame in the input file */
    int width;       // 宽度，感觉和*frame里面的冗余了....
    int height;
    int format;     // 图像或声音格式
    AVRational sar; // 图像宽高比 如果未知或未指定则为0/1
    int uploaded;   // 用来记录该帧是否已经显示过？
    int flip_v;     // =1则旋转180， = 0则正常播放
public:
    Frame() : frame(nullptr), sub(), serial(0), pts(0.0), duration(0.0), pos(0), width(0), height(0), format(0), sar(), uploaded(0), flip_v(0){};
    ~Frame();
};