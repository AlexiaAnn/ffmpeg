#include "ExtractAudioFromMp4.h"
ExtractAudioFromMp4::ExtractAudioFromMp4() {}
const AVCodecID ExtractAudioFromMp4::defaultCodecId = AV_CODEC_ID_MP3;
AVCodecContext* ExtractAudioFromMp4::OpenAudioEncodecContext(AVCodecID codecId)
{
    AVCodecContext* codecContext = AllocEncodecContext(codecId);
    SetAudioEssentialParameters(codecContext, codecId);
    AVCodec* codec = const_cast<AVCodec*>(avcodec_find_encoder(codecId));
    codecContext->sample_fmt = codec->sample_fmts[0];
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec\n");
        return nullptr;
    }
    return codecContext;
}

void ExtractAudioFromMp4::InitAudioFrame(AVFrame*& frame, AVCodecContext* codecCtx, int dstNbSamples)
{
    av_frame_free(&frame);
    frame = AllocAVFrame();
    frame->sample_rate = codecCtx->sample_rate;
    frame->format = codecCtx->sample_fmt;
    av_channel_layout_copy(&frame->ch_layout, &codecCtx->ch_layout);
    frame->nb_samples = dstNbSamples;
    //����buffer
    if ((ret = av_frame_get_buffer(frame, 0)) < 0)
    {
        av_log_error("frame get buffer is failed");
        return;
    }
    if ((ret = av_frame_make_writable(frame)) < 0)
    {
        av_log_error("frame is not writeable");
        return;
    }
}
void ExtractAudioFromMp4::ResampleDeAudioFrame()
{
    int64_t delay = swr_get_delay(swrCtx, deAudioFrame->sample_rate);
    dstNbSamples = av_rescale_rnd(delay + deAudioFrame->nb_samples, enAudioCodecCtx->sample_rate,
        deAudioFrame->sample_rate, AV_ROUND_UP);
    //if (dstNbSamples < 1152) dstNbSamples = 1152;
    av_log_info("delay:%d,deAudioFrame=>sample_rate:%d,nb_samples:%d\n", delay, deAudioFrame->sample_rate, deAudioFrame->nb_samples);
    if (dstNbSamples > maxDstNbSamples)
    {
        InitAudioFrame(enAudioFrame, enAudioCodecCtx, dstNbSamples);
        maxDstNbSamples = dstNbSamples;
    }
    ret = swr_convert(swrCtx, enAudioFrame->data, dstNbSamples,
        const_cast<const uint8_t**>(deAudioFrame->data), deAudioFrame->nb_samples);
    enAudioFrame->nb_samples = ret;
    av_log_info("dstNbSamples:%d,maxDstNbSamples:%d,enAudioFrame sampleRate:%d,nbSamples:%d,sampleFmt:%d\n", 
               dstNbSamples, maxDstNbSamples,enAudioFrame->sample_rate, enAudioFrame->nb_samples, enAudioFrame->format);
    av_log_info("encodeccontext frame_size:%d", enAudioCodecCtx->frame_size);
    if (ret < 0)
    {
        av_log_error("resample is failed\n");
        return;
    }
}
void ExtractAudioFromMp4::EncodeAudioFrame()
{
    ret = avcodec_send_frame(enAudioCodecCtx, enAudioFrame);
    if (ret < 0)
    {
        av_log_error("send frame failed\n");
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enAudioCodecCtx, enAudioPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            av_log_error("Error encoding audio frame");
            return;
        }
        fwrite(enAudioPacket->data, 1, enAudioPacket->size, dstFilePtr);
        av_log_info("write a packet\n");
        av_packet_unref(enAudioPacket);
    }
}
void ExtractAudioFromMp4::DealAudioPacket()
{
    
    ret = avcodec_send_packet(deAudioCodecCtx, dePacket);
    if (ret < 0)
    {
        av_log_error("Error submitting a packet for deconding\n");
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(deAudioCodecCtx, deAudioFrame);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return;

            av_log_error("Error during decoding");
            return;
        }
        ResampleDeAudioFrame();
        EncodeAudioFrame();
        av_frame_unref(deAudioFrame);
        if (ret < 0)
            return;
    }
}
SwrContext* ExtractAudioFromMp4::AllocSwrContext(AVCodecContext* deCodecCtx, AVCodecContext* enCodecCtx)
{
    SwrContext* swrCtx = swr_alloc();
    if (!swrCtx)
    {
        ret = AVERROR(ENOMEM);
        av_log_error("could not allocate resampler context");
        return swrCtx;
    }
    // set swrcontext essential parameters
    av_opt_set_chlayout(swrCtx, "in_chlayout", &(deCodecCtx->ch_layout), 0);
    av_opt_set_int(swrCtx, "in_sample_rate", deCodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", deCodecCtx->sample_fmt, 0);
    av_opt_set_chlayout(swrCtx, "out_chlayout", &(enCodecCtx->ch_layout), 0);
    av_opt_set_int(swrCtx, "out_sample_rate", enCodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", enCodecCtx->sample_fmt, 0);

    //av_log_info("in_sample_fmt:%s,out_sample_fmt:%s",GetSampleFormatString(deCodecCtx->sample_fmt),GetSampleFormatString(enCodecCtx->sample_fmt));
    ret = swr_init(swrCtx);
    if (ret < 0)
    {
        av_log_error("failed to initialize the resampling context\n");
    }
    return swrCtx;
}
SwrContext* ExtractAudioFromMp4::AllocSwrContext(int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout, AVCodecContext* enCodecCtx)
{

    SwrContext* swrCtx = swr_alloc();
    if (!swrCtx)
    {
        ret = AVERROR(ENOMEM);
        av_log_error("could not allocate resampler context");
        return swrCtx;
    }
    // set swrcontext essential parameters
    av_opt_set_chlayout(swrCtx, "in_chlayout", &chLayout, 0);
    av_opt_set_int(swrCtx, "in_sample_rate", sampleRate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", sampleFormat, 0);
    av_opt_set_chlayout(swrCtx, "out_chlayout", &(enCodecCtx->ch_layout), 0);
    av_opt_set_int(swrCtx, "out_sample_rate", enCodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", enCodecCtx->sample_fmt, 0);

    ret = swr_init(swrCtx);
    if (ret < 0)
    {
        av_log_error("failed to initialize the resampling context\n");
    }
    return swrCtx;
}
void ExtractAudioFromMp4::SetAudioEssentialParameters(AVCodecContext*& codecCtx, AVCodecID codecId)
{
    switch (codecId)
    {
    case AV_CODEC_ID_MP3:
    {
        codecCtx->bit_rate = 320000;
        codecCtx->sample_rate = 48000; // error prone
        AVChannelLayout layout = { AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO} };
        av_channel_layout_copy(&codecCtx->ch_layout, &layout);
        break;
    }
    case AV_CODEC_ID_AAC:
    {
        codecCtx->bit_rate = 137000;
        codecCtx->sample_rate = 44100; // error prone
        AVChannelLayout layout = { AV_CHANNEL_ORDER_NATIVE, (2), {AV_CH_LAYOUT_STEREO} };
        av_channel_layout_copy(&codecCtx->ch_layout, &layout);
        break;
    }
    default:
        av_log_error("audio type now is not supported\n");
        ret = -1;
        break;
    }
}
void ExtractAudioFromMp4::WriteFrameEnd()
{
    av_frame_free(&enAudioFrame);
    enAudioFrame = nullptr;
    EncodeAudioFrame();
    fclose(dstFilePtr);
}
ExtractAudioFromMp4::ExtractAudioFromMp4(const char* srcFilePath) : FileContextBase(srcFilePath), dstFilePtr(nullptr), deAudioFrame(nullptr), maxDstNbSamples(0), pts(0)
{
    if (FileContextBase::ret < 0) {
        ret = -1;
        return;
    }
    std::pair<int, int> streamIndexs = getAVStreamIndices(inFmtCtx);
    if (streamIndexs.first < 0) {
        ret = -1;
        return;
    }
    AVStream* inAudioStream = inFmtCtx->streams[streamIndexs.first];
    deAudioCodecCtx = OpenDecodecContextByStream(inAudioStream);
    if (deAudioCodecCtx == nullptr) {
        ret = -1;
        return;
    }
    enCodecId = defaultCodecId;
    enAudioCodecCtx = OpenAudioEncodecContext(defaultCodecId);
    if (enAudioCodecCtx == nullptr) {
        ret = -1;
        return;
    }
    enAudioFrame = AllocAVFrame();
    if (enAudioFrame == nullptr) {
        ret = -1;
        return;
    }
    enAudioPacket = AllocAVPacket();
    if (enAudioPacket == nullptr) {
        ret = -1;
        return;
    }
    swrCtx = AllocSwrContext(deAudioCodecCtx, enAudioCodecCtx);
    if (swrCtx == nullptr) {
        ret = -1;
        return;
    }
}
ExtractAudioFromMp4::ExtractAudioFromMp4(AVCodecID dstCodecID) : FileContextBase(), dstFilePtr(nullptr), swrCtx(nullptr),
deAudioFrame(nullptr), deAudioCodecCtx(nullptr),
enCodecId(dstCodecID), maxDstNbSamples(0), pts(0)
{
    enAudioCodecCtx = OpenAudioEncodecContext(enCodecId);
    enAudioFrame = AllocAVFrame();
    enAudioPacket = AllocAVPacket();
}
ExtractAudioFromMp4::ExtractAudioFromMp4(int sampleRate, AVSampleFormat sampleFormat, AVChannelLayout chLayout) : FileContextBase(), dstFilePtr(nullptr),
deAudioFrame(nullptr), deAudioCodecCtx(nullptr),
enCodecId(defaultCodecId), maxDstNbSamples(0), pts(0),
dstNbSamples(0)
{
    enAudioCodecCtx = OpenAudioEncodecContext(enCodecId);
    enAudioFrame = AllocAVFrame();
    enAudioPacket = AllocAVPacket();
    swrCtx = AllocSwrContext(sampleRate, sampleFormat, chLayout, enAudioCodecCtx);
}
bool ExtractAudioFromMp4::ExtractAudioToFile(const char* srcFilePath, const char* dstFilePath)
{
    if (ret < 0)
    {
        av_log_error("construct error\n");
        return false;
    }
    // if (enCodecId != AV_CODEC_ID_MP3)
    // {
    //     av_log_error("enCodecId!=AV_CODEC_ID_MP3,please reset enCodecId\n");
    //     return false;
    // }
    // set contextbase members
    ResetFormatContextByFileName(srcFilePath);
    if (dePacket == nullptr)
        dePacket = AllocAVPacket();
    // set decodeccontext
    int audioStreamIndex = av_find_best_stream(inFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    AVStream* audioStream = inFmtCtx->streams[audioStreamIndex];
    AVCodecID deCodecId = audioStream->codecpar->codec_id;
    if (deAudioCodecCtx == nullptr)
    {
        deAudioCodecCtx = OpenDecodecContextByStream(audioStream);
        av_log_info("deCodecContext is nullptr,now open the deCodecContext\n");
    }
    else
    {
        if (deCodecId != deAudioCodecCtx->codec_id)
        {
            // reset decodeccontxt
            avcodec_free_context(&deAudioCodecCtx);
            deAudioCodecCtx = OpenDecodecContextByStream(audioStream);
            av_log_info("deCodecContext need to change,reopen the deCodecContext\n");
        }
    }
    // set swrcontext
    if (swrCtx == nullptr)
    {
        swrCtx = AllocSwrContext(deAudioCodecCtx, enAudioCodecCtx);
        av_log_info("swrcontext is nullptr,alloced swrcontext\n");
    }
    else
    {
        // reset swrcontext
        // todo: is need reset swrcontext
        swr_free(&swrCtx);
        swrCtx = AllocSwrContext(deAudioCodecCtx, enAudioCodecCtx);
        av_log_info("swrcontext need to change,reset swrcontext\n");
    }
    deAudioFrame = AllocAVFrame();
    // start decode and write to dst file
    dstFilePtr = fopen(dstFilePath, "wb");
    if (dstFilePtr == nullptr)
    {
        ret = -1;
        av_log_error("%s can not open\n", dstFilePath);
        return false;
    }
    DecodePakcet();
    AVFrame* temp = enAudioFrame;
    enAudioFrame = nullptr;
    EncodeAudioFrame();
    enAudioFrame = temp;
    fclose(dstFilePtr);
    av_frame_free(&deAudioFrame);
    return ret >= 0;
}
void ExtractAudioFromMp4::ResetAudioCodecId(AVCodecID codecId)
{
    if (codecId == enCodecId)
    {
        av_log_info("AVCodecID is not change\n");
        return;
    }
    enCodecId = codecId;
    avcodec_free_context(&enAudioCodecCtx);
    enAudioCodecCtx = OpenAudioEncodecContext(enCodecId);
}
void ExtractAudioFromMp4::FlushEnAudioCodecBuffer()
{
    av_frame_free(&enAudioFrame);
    enAudioFrame = nullptr;
    EncodeAudioFrame();
}
void ExtractAudioFromMp4::PcmToAACFile(const char* srcFilePath, const char* dstFilepath)
{
    enAudioFrame->ch_layout = enAudioCodecCtx->ch_layout;
    enAudioFrame->sample_rate = enAudioCodecCtx->sample_rate;
    enAudioFrame->nb_samples = 1024;
    enAudioFrame->format = enAudioCodecCtx->sample_fmt;
    enAudioFrame->pts = 0;
    dstFilePtr = fopen(dstFilepath, "wb");
    FILE* srcFilePtr = fopen(srcFilePath, "rb");
    int size = av_samples_get_buffer_size(nullptr, 2, 1024, AV_SAMPLE_FMT_FLTP, 0);
    uint8_t* data = new uint8_t(size);
    while (true)
    {
        int len = fread(data, 1, size, srcFilePtr);
        if (len <= 0)
            break;
        enAudioFrame->data[0] = data;
        enAudioFrame->data[1] = data + size / 2;
        enAudioFrame->pts += 1024;
        EncodeAudioFrame();
    }
    av_frame_free(&enAudioFrame);
    enAudioFrame = nullptr;
    EncodeAudioFrame();
    fclose(dstFilePtr);
    delete data;
}
ExtractAudioFromMp4::~ExtractAudioFromMp4()
{
    swr_free(&swrCtx);
    av_frame_free(&enAudioFrame);
    av_packet_free(&enAudioPacket);
    avcodec_free_context(&deAudioCodecCtx);
    avcodec_free_context(&enAudioCodecCtx);
}