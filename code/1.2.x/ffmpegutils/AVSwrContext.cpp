#include "AVSwrContext.h"

void AVSwrContext::ReAllocFrame(AVFrame*& frame,int dstNbSamples)
{
	int format = frame->format;
	AVChannelLayout layout = frame->ch_layout;
	int sampleRate = frame->sample_rate;
	av_frame_free(&frame);
	frame = AllocAVFrame();
	frame->sample_rate = sampleRate;
	frame->format = format;
	av_channel_layout_copy(&frame->ch_layout, &layout);
	frame->nb_samples = dstNbSamples;
	// alloc buffer
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

AVSwrContext::AVSwrContext():swrCont(nullptr),ret(0)
{
}

AVSwrContext::AVSwrContext(int deSampleRate, AVSampleFormat deSampleFormat, AVChannelLayout deChLayout,
						   int enSampleRate, AVSampleFormat enSampleFormat, AVChannelLayout enChLayout)
{
	swrCont = swr_alloc();
	if (swrCont == nullptr) {
		ret = AVERROR(ENOMEM);
		av_log_error("could not allocate resampler context\n");
		return;
	}
	// set swrcontext essential parameters
	av_opt_set_chlayout(swrCont, "in_chlayout", &deChLayout, 0);
	av_opt_set_int(swrCont, "in_sample_rate", deSampleRate, 0);
	av_opt_set_sample_fmt(swrCont, "in_sample_fmt", deSampleFormat, 0);
	av_opt_set_chlayout(swrCont, "out_chlayout", &enChLayout, 0);
	av_opt_set_int(swrCont, "out_sample_rate", enSampleRate, 0);
	av_opt_set_sample_fmt(swrCont, "out_sample_fmt", enSampleFormat, 0);
	ret = swr_init(swrCont);
	if (ret < 0)
	{
		swr_free(&swrCont);
		swrCont = nullptr;
		av_log_error("failed to initialize the resampling context\n");
		return;
	}
	av_log_info("AVSwrContext initialize success\n");
}

AVSwrContext::AVSwrContext(AVCodecContext* deAudioCodecCont, 
						   int enSampleRate, AVSampleFormat enSampleFormat, AVChannelLayout enChLayout):
						   AVSwrContext(deAudioCodecCont->sample_rate, deAudioCodecCont->sample_fmt,deAudioCodecCont->ch_layout,
										enSampleRate,enSampleFormat, enChLayout)
{
}

AVSwrContext::AVSwrContext(int deSampleRate, AVSampleFormat deSampleFormat, AVChannelLayout deChLayout, 
						   AVCodecContext* enAudioCodecCont):
						   AVSwrContext(deSampleRate,deSampleFormat,deChLayout,
							            enAudioCodecCont->sample_rate,enAudioCodecCont->sample_fmt,enAudioCodecCont->ch_layout)
{

}

AVSwrContext::AVSwrContext(AVCodecContext* deAudioCodecCont, AVCodecContext* enAudioCodecCont):
			  AVSwrContext(deAudioCodecCont->sample_rate, deAudioCodecCont->sample_fmt, deAudioCodecCont->ch_layout,
		                   enAudioCodecCont->sample_rate, enAudioCodecCont->sample_fmt, enAudioCodecCont->ch_layout)
{

}

bool AVSwrContext::ResampleAudioFrame(AVFrame* deFrame, AVFrame*& enFrame)
{
	//check
	if (swrCont == nullptr) {
		av_log_warning("swrcontext is nullptr,cant resample audio frame\n");
		return false;
	}

	int64_t delay = swr_get_delay(swrCont, deFrame->sample_rate);
	int dstNbSamples = av_rescale_rnd(delay + deFrame->nb_samples, enFrame->sample_rate,
									  deFrame->sample_rate, AV_ROUND_UP);
	if (dstNbSamples > enFrame->nb_samples) {
		ReAllocFrame(enFrame,dstNbSamples);
	}
	//resample
	ret = swr_convert(swrCont, enFrame->data, dstNbSamples,
		const_cast<const uint8_t**>(deFrame->data), deFrame->nb_samples);
	enFrame->nb_samples = ret; // error prone
	//av_log_info("resample success,EnAudio Frame sample number:%d\n", ret);
	if (ret < 0)
	{
		av_log_error("resample is failed\n");
		return false;
	}
	return true;
}

bool AVSwrContext::ResampleAudioFrame(AVFrame* deFrame, EnCodecAudioContext& codeCont)
{
	//check=>whether swrcontext was nullptr
	if (swrCont == nullptr) {
		av_log_warning("swrcontext is nullptr,cant resample audio frame\n");
		return false;
	}

	AVFrame* enFrame = codeCont.GetEncodecFrame();
	if (enFrame == nullptr) {
		av_log_warning("enframe is nullptr,cant to resample");
		return false;
	}
	//get dst sample number aboud source frame to target frame
	int64_t delay = swr_get_delay(swrCont, deFrame->sample_rate);
	int dstNbSamples = av_rescale_rnd(delay + deFrame->nb_samples, enFrame->sample_rate,
		deFrame->sample_rate, AV_ROUND_UP);
	if (dstNbSamples > codeCont.GetNbSamplesOfFrameBuffer()) {
		codeCont.ReAllocFrame(dstNbSamples);
		enFrame = codeCont.GetEncodecFrame();
	}
	
	//resample
	ret = swr_convert(swrCont, enFrame->data, dstNbSamples,
		const_cast<const uint8_t**>(deFrame->data), deFrame->nb_samples);
	enFrame->nb_samples = ret;
	//av_log_info("resample success,EnAudio Frame sample number:%d\n", ret);
	//whether resampling was successful
	if (ret < 0)
	{
		av_log_error("resample is failed\n");
		return false;
	}
	return true;
}

int AVSwrContext::GetResult() const
{
	return ret;
}

AVSwrContext::~AVSwrContext()
{
	swr_free(&swrCont);
	swrCont = nullptr;
}
