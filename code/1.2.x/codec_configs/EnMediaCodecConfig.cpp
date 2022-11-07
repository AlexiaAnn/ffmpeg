#include "EnMediaCodecConfig.h"

EnMediaCodecConfig::EnMediaCodecConfig(int width, int height, int fps, EncodeName encodeName, 
                                       int in_timeout, int out_timtout, int in_timeout_times, int out_timeout_times):
                                       VideoEnCodecConfigBase(width, height, fps, encodeName)
{
    this->in_timeout = in_timeout;
    this->out_timeout = out_timtout;
    this->in_timeout_times = in_timeout_times;
    this->out_timeout_times = out_timeout_times;
}

AVCodecContext* EnMediaCodecConfig::GetContext()
{
    AVCodecContext* context = VideoEnCodecConfigBase::GetContext();
    if (context == nullptr) return nullptr;

    av_opt_set_int(context->priv_data, "in_timeout", in_timeout, 0);
    av_opt_set_int(context->priv_data, "out_timeout", out_timeout, 0);
    av_opt_set_int(context->priv_data, "in_timeout_times", in_timeout_times, 0);
    av_opt_set_int(context->priv_data, "out_timeout_times", out_timeout_times, 0);

    const AVCodec* codec = avcodec_find_encoder_by_name(encodeNames[int(encodeName)].c_str());
    int ret = avcodec_open2(context, codec, nullptr);
    if (ret < 0)
    {
        av_log_error("could not open codec context\n");
        return nullptr;
    }
    av_log_info("open video codec context end\n");
    return context;
}
