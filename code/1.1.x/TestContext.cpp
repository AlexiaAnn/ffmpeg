#include "TestContext.h"

TestContext::TestContext(const char* srcFilePath)
{
	context = avformat_alloc_context();
	if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
		av_log_error("scan_all_pmts failed,set it 1\n");
		av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
		scan_all_pmts_set = 1;
	}
	ret = avformat_open_input(&context, srcFilePath, nullptr,&format_opts);
	if (ret < 0) {
		av_log_error("open formatcontext failed\n");
		return;
	}
	if (scan_all_pmts_set)
		av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);
	if ((t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
		av_log_error("Option %s not found.\n", t->key);
		ret = AVERROR_OPTION_NOT_FOUND;
		return;
	}
	context->flags |= AVFMT_FLAG_GENPTS;

	av_format_inject_global_side_data(context);

//	AVDictionary** opts = setup_find_stream_info_opts(context, codec_opts);
	int orig_nb_streams = context->nb_streams;

	ret = avformat_find_stream_info(context, nullptr);

	//for (int i = 0; i < orig_nb_streams; i++)
	//	av_dict_free(&opts[i]);
	//av_freep(&opts);

	if (ret < 0) {
		av_log(NULL, AV_LOG_WARNING,
			"%s: could not find codec parameters\n", context->url);
		ret = -1;
		return;
	}

	packet = av_packet_alloc();
}

void TestContext::main()
{
	if (ret < 0)return;
	av_dump_format(context,0,context->url,0);
	for (int i = 0; i < context->nb_streams; ++i) {
		AVStream* stream = context->streams[i];
		int fps = stream->r_frame_rate.den == 0 ? 0 : stream->r_frame_rate.num / stream->r_frame_rate.den;
		av_log_info("stream_index:%d,duration:%d,nb_frames:%d,bitrate:%d,fps:%d\n",
			i,stream->duration,stream->nb_frames,context->bit_rate,fps);
	}
	//while (av_read_frame(context, packet)>=0) {
	//	av_log_info("%d ",packet->stream_index);
	//}
	int nb_frames = av_rescale_q(context->duration, { 1,AV_TIME_BASE }, {1,30});
	av_log_info("context duration:%d,nb_frames:%d",context->duration,nb_frames);
}
