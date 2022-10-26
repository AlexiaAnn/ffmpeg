#include "FilterContext.h"

FilterContext::FilterContext(AVCodecContext* codeCont)
{
	char args[512];
	AVPixelFormat pix_fmts[2] = { DEFAULTGIFPIXFMT, AV_PIX_FMT_NONE };
	const AVFilter* buffersrc=nullptr, *buffersink=nullptr;
	const char* format = "format=pix_fmts=rgb565,fps=%d,split [o1] [o2];[o1] palettegen=stats_mode=diff [p]; [o2] [p] paletteuse=dither=bayer:bayer_scale=5:diff_mode=rectangle";
	char* filter_desc = (char*)malloc(strlen(format) + 10);
	sprintf(filter_desc, format, codeCont->framerate.num);

	AVFilterInOut* inputs = AllocAVFilterInOut();
	AVFilterInOut* outputs = AllocAVFilterInOut();
	filterGraph = AllocAVFilterGraph();
	if (inputs == nullptr || outputs == nullptr || filterGraph == nullptr) goto end;

	snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		codeCont->width, codeCont->height, DEFAULTGIFINPUTPIXFMT,
		codeCont->time_base.num, codeCont->time_base.den,
		codeCont->sample_aspect_ratio.num, codeCont->sample_aspect_ratio.den);

	buffersrc = avfilter_get_by_name("buffer");
	buffersink = avfilter_get_by_name("buffersink");
	if (buffersrc == nullptr || buffersink == nullptr) goto end;

	//创建滤波器的上下文
	ret = avfilter_graph_create_filter(&buffersrcCont, buffersrc, "in", args, nullptr, filterGraph);
	if (ret < 0)
	{
		av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer source\n");
		goto end;
	}
	ret = avfilter_graph_create_filter(&buffersinkCont, buffersink, "out", nullptr, nullptr, filterGraph);
	if (ret < 0)
	{
		av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer sink\n");
		goto end;
	}
	//给滤波器设置参数
	ret = av_opt_set_int_list(buffersinkCont, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
	if (ret < 0)
	{
		av_log(nullptr, AV_LOG_ERROR, "can not set output pixel format\n");
		goto end;
	}

	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrcCont;
	outputs->pad_idx = 0;
	outputs->next = nullptr;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersinkCont;
	inputs->pad_idx = 0;
	inputs->next = nullptr;

	//设置滤波器图的描述字符串和输入端，输出端
	if ((ret = avfilter_graph_parse_ptr(filterGraph, filter_desc, &inputs, &outputs, nullptr)) < 0)
	{
		av_log_error("parse filter graph error\n");
		goto end;
	}

	//检查FilterGraph的配置。
	if ((ret = avfilter_graph_config(filterGraph, nullptr)) < 0)
	{
		av_log_error("config graph error\n");
		goto end;
	}

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

	sinkFrame = AllocAVFrame();
	if (sinkFrame == nullptr) goto end;
	return;
end:
	ret = -1;
	return;
}

bool FilterContext::AddFrame(AVFrame* frame)
{
	ret = av_buffersrc_add_frame_flags(buffersrcCont,frame, AV_BUFFERSRC_FLAG_PUSH);
	return ret >= 0;
}

bool FilterContext::FlushBuffer()
{
	ret = av_buffersrc_add_frame_flags(buffersrcCont, nullptr, AV_BUFFERSRC_FLAG_PUSH);
	return ret >= 0;
}

AVFrame* FilterContext::GetFrame()
{
	ret = av_buffersink_get_frame(buffersinkCont,sinkFrame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return nullptr;
	return sinkFrame;
}

int FilterContext::GetResult() const
{
	return ret;
}

FilterContext::~FilterContext()
{
	avfilter_free(buffersrcCont);
	buffersrcCont = nullptr;
	avfilter_free(buffersinkCont);
	buffersinkCont = nullptr;
	avfilter_graph_free(&filterGraph);
	filterGraph = nullptr;
}
