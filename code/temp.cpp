void gifEncode()
{
    char *path = "d:/new_test.gif";
    int fps = 25;

    //注册组件
    av_register_all();

    int width = 1920;
    int height = 1080;

    AVFormatContext *avformat_context = NULL;
    //初始化封装格式上下文
    avformat_alloc_output_context2(&avformat_context, NULL, NULL, path);

    //打开输出文件
    if (avio_open(&avformat_context->pb, path, AVIO_FLAG_WRITE) < 0)
    {
        return;
    }
    //查找编码器
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_GIF);
    //创建编码器的上下文
    AVCodecContext *avcodec_context = avcodec_alloc_context3(codec);
    //时间基,pts,dts的时间单位  pts(解码后帧被显示的时间), dts(视频帧送入解码器的时间)的时间单位,是两帧之间的时间间隔
    avcodec_context->time_base.den = fps; // pts
    avcodec_context->time_base.num = 1;   // 1秒
    avcodec_context->codec_id = AV_CODEC_ID_GIF;

    avcodec_context->codec_type = AVMEDIA_TYPE_VIDEO; //表示视频类型
    avcodec_context->pix_fmt = AV_PIX_FMT_PAL8;       //视频数据像素格式

    avcodec_context->width = width; //视频宽高
    avcodec_context->height = height;

    //在gif文件中
    AVStream *avvideo_stream = avformat_new_stream(avformat_context, NULL); //创建一个流
    avvideo_stream->codec = avcodec_context;
    avvideo_stream->time_base = avcodec_context->time_base;
    avvideo_stream->codec->codec_tag = 0;

    //初始化编解码器
    int ret = avcodec_open2(avcodec_context, codec, nullptr);
    if (ret)
    {
        return;
    }

    int avformat_write_header_result = avformat_write_header(avformat_context, NULL);
    if (avformat_write_header_result != AVSTREAM_INIT_IN_WRITE_HEADER)
    {
        return;
    }
    //这个滤镜字符串的大概意思是设置输入的视频帧格式和fps,然后将帧分成两份o1, o2, 使用palettegen滤镜将o1生成gif调色板p,  最后使用paletteuse滤镜、利用调色板p采样，将o2生成为gif
    char *format = "format=pix_fmts=rgb565,fps=%d,split [o1] [o2];[o1] palettegen=stats_mode=diff [p]; [o2] [p] paletteuse=dither=bayer:bayer_scale=5:diff_mode=rectangle";
    char *str = (char *)malloc(strlen(format) + 10);
    sprintf(str, format, 25);

    AVFilterGraph *filter_graph;
    AVFilterContext *buffersrc_ctx;
    AVFilterContext *buffersink_ctx;

    initFilter(avcodec_context, &filter_graph, &buffersrc_ctx, &buffersink_ctx, str);

    int buffer_size = av_image_get_buffer_size(avcodec_context->pix_fmt,
                                               avcodec_context->width,
                                               avcodec_context->height,
                                               1);
    uint8_t *out_buffer = (uint8_t *)av_malloc(buffer_size);

    AVFrame *frame = av_frame_alloc();

    AVPacket *av_packet = av_packet_alloc();
    av_init_packet(av_packet);

    uint8_t *file_buffer = (uint8_t *)av_malloc(width * height * 3 / 2);

    FILE *in_file = fopen("d:/yuv4201.yuv", "rb");
    int i = 0;

    SwsContext *swsContext = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width, height, AV_PIX_FMT_RGB565, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    if (swsContext == nullptr)
    {
        return;
    }

    AVFrame *rgbFrame = av_frame_alloc();
    rgbFrame->linesize[0] = width * 2;
    rgbFrame->format = AV_PIX_FMT_RGB565;
    rgbFrame->width = width;
    rgbFrame->height = height;
    unsigned char *out_buffer1 = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB565, avcodec_context->width, avcodec_context->height));
    avpicture_fill((AVPicture *)rgbFrame, out_buffer1, AV_PIX_FMT_RGB565, avcodec_context->width, avcodec_context->height);

    while (true)
    {

        //读取yuv帧数据  注意yuv420p的长度  width * height * 3 / 2
        if (fread(file_buffer, 1, width * height * 3 / 2, in_file) <= 0)
        {
            break;
        }
        else if (feof(in_file))
        {
            break;
        }

        //封装yuv帧数据
        frame->data[0] = file_buffer;
        frame->data[1] = file_buffer + width * height;
        frame->data[2] = file_buffer + width * height * 5 / 4;
        frame->linesize[0] = width;
        frame->linesize[1] = width / 2;
        frame->linesize[2] = width / 2;
        // frame->pts = i;
        frame->pts = i;
        frame->width = width;
        frame->height = height;
        i++;

        sws_scale(
            swsContext,
            (uint8_t const *const *)frame->data,
            frame->linesize,
            0,
            height,
            rgbFrame->data,
            rgbFrame->linesize);
        rgbFrame->pts = i;
        if (i == 1)
        {
            continue;
        }
        // 往源滤波器buffer中输入待处理的数据
        av_buffersrc_add_frame_flags(buffersrc_ctx, rgbFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
    }

    sws_freeContext(swsContext);

    // 最后写入一个空帧
    if (av_buffersrc_add_frame_flags(buffersrc_ctx, nullptr, AV_BUFFERSRC_FLAG_KEEP_REF) >= 0)
    {
        do
        {
            AVFrame *gifFrame = av_frame_alloc();
            // 从目的滤波器buffersink中输出处理完的数据
            int ret = av_buffersink_get_frame(buffersink_ctx, gifFrame);
            //                    ret = av_buffersink_get_frame(buffersink_ctx, filter_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                // av_log(nullptr, AV_LOG_ERROR, "error get frame from buffer sink %s\n", av_err2str(ret));
                break;
            }

            // write the filter frame to output file
            ret = avcodec_send_frame(avcodec_context, gifFrame);
            AVPacket *pkt = av_packet_alloc();
            av_init_packet(pkt);

            while (ret >= 0)
            {
                ret = avcodec_receive_packet(avcodec_context, pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }

                AVRational codecTimebase = avcodec_context->time_base;

                AVRational streamTimebase = avformat_context->streams[0]->time_base;

                av_packet_rescale_ts(pkt, codecTimebase, streamTimebase);

                // pkt.RescaleTs(encoderTimebase, streamTimebase);

                av_write_frame(avformat_context, pkt);
            }

            av_packet_unref(pkt);

        } while (ret >= 0);
    }

    fclose(in_file);
    avcodec_close(avcodec_context);
    av_free(frame);
    av_free(out_buffer);
    av_free_packet(av_packet);

    av_write_trailer(avformat_context);
    avio_close(avformat_context->pb);
    avformat_free_context(avformat_context);

    avfilter_free(buffersrc_ctx);
    avfilter_free(buffersink_ctx);
    avfilter_graph_free(&filter_graph);
}

int initFilter(AVCodecContext *context, AVFilterGraph **filter_graph, AVFilterContext **buffersrc_ctx, AVFilterContext **buffersink_ctx, const char *filter_desc)
{
    char args[512];
    int ret = 0;

    //分别获取输入和输出的滤波器
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");

    //创建滤波器的输入端
    AVFilterInOut *inputs = avfilter_inout_alloc();
    //创建滤波器的输出端
    AVFilterInOut *outputs = avfilter_inout_alloc();
    // MBAVRational avrational;
    // encoder->GetTimeBase(avrational);

    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_PAL8, AV_PIX_FMT_NONE};

    // 创建一个滤波器图filter graph
    *filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !*filter_graph)
    {
        ret = AVERROR(ENOMEM);
        return ret;
    }

    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             context->width, context->height, AV_PIX_FMT_RGB565,
             context->time_base.num, context->time_base.den,
             context->sample_aspect_ratio.num, context->sample_aspect_ratio.den);

    //创建滤波器的上下文
    ret = avfilter_graph_create_filter(buffersrc_ctx, buffersrc, "in", args, nullptr, *filter_graph);
    if (ret < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer source\n");
        return ret;
    }

    ret = avfilter_graph_create_filter(buffersink_ctx, buffersink, "out", nullptr, nullptr, *filter_graph);
    if (ret < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer sink\n");
        return ret;
    }

    //给滤波器设置参数
    av_opt_set_int_list(*buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "can not set output pixel format\n");
        return ret;
    }

    outputs->name = av_strdup("in");
    outputs->filter_ctx = *buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = *buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    //设置滤波器图的描述字符串和输入端，输出端
    if ((ret = avfilter_graph_parse_ptr(*filter_graph, filter_desc, &inputs, &outputs, nullptr)) < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "parse filter graph error\n");
        return ret;
    }

    //检查FilterGraph的配置。
    if ((ret = avfilter_graph_config(*filter_graph, nullptr)) < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "config graph error\n");
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return 0;
}
