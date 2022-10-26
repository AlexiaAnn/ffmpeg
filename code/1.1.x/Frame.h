#pragma once
#include "IncludeFFmpeg.h"
class Frame
{
public:
    AVFrame *frame;
    AVSubtitle sub;  // ��Ļ
    int serial;      // ��������
    double pts;      // ��ǰָ֡���pts ʱ�������λΪ��       /* presentation timestamp for the frame */
    double duration; // ��ǰ֡��������ʱ��    /* estimated duration of the frame */
    int64_t pos;     // ��֡���ļ��е��ֽ�λ��    /* byte position of the frame in the input file */
    int width;       // ��ȣ��о���*frame�����������....
    int height;
    int format;     // ͼ���������ʽ
    AVRational sar; // ͼ���߱� ���δ֪��δָ����Ϊ0/1
    int uploaded;   // ������¼��֡�Ƿ��Ѿ���ʾ����
    int flip_v;     // =1����ת180�� = 0����������
public:
    Frame() : frame(nullptr), sub(), serial(0), pts(0.0), duration(0.0), pos(0), width(0), height(0), format(0), sar(), uploaded(0), flip_v(0){};
    ~Frame();
};