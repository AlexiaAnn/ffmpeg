#include "AudioWave.h"

//int AudioWave::InitFilterGraph(AVSampleFormat sampleFormat, int sampleRate)
//{
//    AVFilterGraph* filter_graph;
//    AVFilterContext* abuffer_ctx;
//    const AVFilter* abuffer;
//    AVFilterContext* volume_ctx;
//    const AVFilter* volume;
//    AVFilterContext* aformat_ctx;
//    const AVFilter* aformat;
//    AVFilterContext* abuffersink_ctx;
//    const AVFilter* abuffersink;
//
//    AVDictionary* options_dict = NULL;
//    char options_str[1024];
//    char ch_layout[64];
//
//    int err;
//
//    /* Create a new filtergraph, which will contain all the filters. */
//    filter_graph = avfilter_graph_alloc();
//    if (!filter_graph) {
//        fprintf(stderr, "Unable to create filter graph.\n");
//        return AVERROR(ENOMEM);
//    }
//
//    /* Create the abuffer filter;
//     * it will be used for feeding the data into the graph. */
//    abuffer = avfilter_get_by_name("abuffer");
//    if (!abuffer) {
//        fprintf(stderr, "Could not find the abuffer filter.\n");
//        return AVERROR_FILTER_NOT_FOUND;
//    }
//
//    abuffer_ctx = avfilter_graph_alloc_filter(filter_graph, abuffer, "src");
//    if (!abuffer_ctx) {
//        fprintf(stderr, "Could not allocate the abuffer instance.\n");
//        return AVERROR(ENOMEM);
//    }
//
//    /* Set the filter options through the AVOptions API. */
//    av_channel_layout_describe(&deAudioCodecCtx->ch_layout, ch_layout, sizeof(ch_layout));
//    av_opt_set(abuffer_ctx, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);
//    av_opt_set(abuffer_ctx, "sample_fmt", av_get_sample_fmt_name(INPUT_FORMAT), AV_OPT_SEARCH_CHILDREN);
//    av_opt_set_q(abuffer_ctx, "time_base", { 1, INPUT_SAMPLERATE }, AV_OPT_SEARCH_CHILDREN);
//    av_opt_set_int(abuffer_ctx, "sample_rate", INPUT_SAMPLERATE, AV_OPT_SEARCH_CHILDREN);
//
//    /* Now initialize the filter; we pass NULL options, since we have already
//     * set all the options above. */
//    err = avfilter_init_str(abuffer_ctx, NULL);
//    if (err < 0) {
//        fprintf(stderr, "Could not initialize the abuffer filter.\n");
//        return err;
//    }
//
//    /* Create volume filter. */
//    volume = avfilter_get_by_name("volume");
//    if (!volume) {
//        fprintf(stderr, "Could not find the volume filter.\n");
//        return AVERROR_FILTER_NOT_FOUND;
//    }
//
//    volume_ctx = avfilter_graph_alloc_filter(filter_graph, volume, "volume");
//    if (!volume_ctx) {
//        fprintf(stderr, "Could not allocate the volume instance.\n");
//        return AVERROR(ENOMEM);
//    }
//
//    /* A different way of passing the options is as key/value pairs in a
//     * dictionary. */
//    av_dict_set(&options_dict, "volume", AV_STRINGIFY(VOLUME_VAL), 0);
//    err = avfilter_init_dict(volume_ctx, &options_dict);
//    av_dict_free(&options_dict);
//    if (err < 0) {
//        fprintf(stderr, "Could not initialize the volume filter.\n");
//        return err;
//    }
//
//    /* Create the aformat filter;
//     * it ensures that the output is of the format we want. */
//    aformat = avfilter_get_by_name("aformat");
//    if (!aformat) {
//        fprintf(stderr, "Could not find the aformat filter.\n");
//        return AVERROR_FILTER_NOT_FOUND;
//    }
//
//    aformat_ctx = avfilter_graph_alloc_filter(filter_graph, aformat, "aformat");
//    if (!aformat_ctx) {
//        fprintf(stderr, "Could not allocate the aformat instance.\n");
//        return AVERROR(ENOMEM);
//    }
//
//    /* A third way of passing the options is in a string of the form
//     * key1=value1:key2=value2.... */
//    snprintf(options_str, sizeof(options_str),
//        "sample_fmts=%s:sample_rates=%d:channel_layouts=stereo",
//        av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), 44100);
//    err = avfilter_init_str(aformat_ctx, options_str);
//    if (err < 0) {
//        av_log(NULL, AV_LOG_ERROR, "Could not initialize the aformat filter.\n");
//        return err;
//    }
//
//    /* Finally create the abuffersink filter;
//     * it will be used to get the filtered data out of the graph. */
//    abuffersink = avfilter_get_by_name("abuffersink");
//    if (!abuffersink) {
//        fprintf(stderr, "Could not find the abuffersink filter.\n");
//        return AVERROR_FILTER_NOT_FOUND;
//    }
//
//    abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "sink");
//    if (!abuffersink_ctx) {
//        fprintf(stderr, "Could not allocate the abuffersink instance.\n");
//        return AVERROR(ENOMEM);
//    }
//
//    /* This filter takes no options. */
//    err = avfilter_init_str(abuffersink_ctx, NULL);
//    if (err < 0) {
//        fprintf(stderr, "Could not initialize the abuffersink instance.\n");
//        return err;
//    }
//
//    /* Connect the filters;
//     * in this simple case the filters just form a linear chain. */
//    err = avfilter_link(abuffer_ctx, 0, volume_ctx, 0);
//    if (err >= 0)
//        err = avfilter_link(volume_ctx, 0, aformat_ctx, 0);
//    if (err >= 0)
//        err = avfilter_link(aformat_ctx, 0, abuffersink_ctx, 0);
//    if (err < 0) {
//        fprintf(stderr, "Error connecting filters\n");
//        return err;
//    }
//
//    /* Configure the graph. */
//    err = avfilter_graph_config(filter_graph, NULL);
//    if (err < 0) {
//        av_log(NULL, AV_LOG_ERROR, "Error configuring the filter graph\n");
//        return err;
//    }
//
//    graph = filter_graph;
//    src = abuffer_ctx;
//    sink = abuffersink_ctx;
//
//    return 0;
//}

AudioWave::AudioWave(const char* srcFilePath) :FileContextBase(srcFilePath),audioStream(nullptr)
{
	DumpFormatContextInfo(srcFilePath);
	if (inFmtCtx->duration < 0) {
		av_log_info("audio duration:%ld\n", inFmtCtx->duration);
		ret = -1;
		return;
	}
	for (int i = 0; i < inFmtCtx->nb_streams; ++i) {
		if (inFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = inFmtCtx->streams[i];
			break;
		}
	}
	if (audioStream == nullptr) {
		ret = -1;
		return;
	}
	if (audioStream->codecpar->sample_rate <= 0) {
		ret = -1;
		return;
	}
	deAudioFrame = AllocAVFrame();
	deAudioCodecCtx = OpenDecodecContextByStream(audioStream);
	AVSampleFormat format = (AVSampleFormat)audioStream->codecpar->format;
	if (!(format == AV_SAMPLE_FMT_FLT || format == AV_SAMPLE_FMT_FLTP)) {
		enAudioCodecCtx = avcodec_alloc_context3(nullptr);
		enAudioCodecCtx->sample_rate = deAudioCodecCtx->sample_rate;
		enAudioCodecCtx->ch_layout = deAudioCodecCtx->ch_layout;
		enAudioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
		swrCtx = InitSwrContext(deAudioCodecCtx, enAudioCodecCtx);
	}
	//result = new float[DEFAULTPOINTNUMBER+1];
	//ret = InitFilterGraph(AV_SAMPLE_FMT_FLT,audioStream->duration/DEFAULTPOINTNUMBER);
	
}

int AudioWave::GetMetaDataLength() const
{
	return DEFAULTPOINTNUMBER + 1;
}

double AudioWave::GetSecondsOfDuration() const
{
	return (double)inFmtCtx->duration / (double)AV_TIME_BASE;
}

int AudioWave::GetAudioSampleRate() const
{
	return audioStream->codecpar->sample_rate;
}

int AudioWave::GetAudioBitRate() const
{
	return audioStream->codecpar->bit_rate;
}

void AudioWave::GetMetaData(float* result)
{
	AVFrame* frame = nullptr;
	int samples = int((double)inFmtCtx->duration / AV_TIME_BASE * audioStream->codecpar->sample_rate);
	const int interval = samples / DEFAULTPOINTNUMBER;
	int i = 0;
	int resultIndex = 0;
	while (frame = GetNextAudioFrame()) {
		if (swrCtx != nullptr) {
			ResampleDeAudioFrame();
			frame = enAudioFrame;
		}
		float* data = (float*)frame->data[0];
		for (; i < frame->nb_samples; i += interval, ++resultIndex) {
			*(result + resultIndex) = *(data + i);
		}
		i -= frame->nb_samples;
		*(result + DEFAULTPOINTNUMBER) = *(data + frame->nb_samples - 1);
	}
}

int AudioWave::GetRet() const
{
	return ret;
}

void AudioWave::GetAudioInformation()
{
	float* data = new float[GetMetaDataLength()];
	GetMetaData(data);
	for (int i = 0; i < GetMetaDataLength(); ++i) {
		
			av_log_info("%f ",*(data+i));
	}
	delete[] data;
}

AudioWave::~AudioWave()
{

}
