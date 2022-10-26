#include "NewPlayer.h"

void NewPlayer::InitData()
{
    wanted_stream_spec[AVMEDIA_TYPE_VIDEO] = "select desired video stream";
    wanted_stream_spec[AVMEDIA_TYPE_AUDIO] = "select desired audio stream";
    wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE] = "select desired subtitle stream";
    last_audio_stream = video_stream = -1;
    last_audio_stream = audio_stream = -1;
    last_subtitle_stream = subtitle_stream = -1;
    // iformat = const_cast<AVInputFormat *>(inFmtCtx->iformat);
    ytop = 0;
    xleft = 0;

    // frame queue
    int ret = pictq.FrameQueueInit(&videoq, VIDEO_PICTURE_QUEUE_SIZE, 1);
    if (ret < 0)
        return;
    ret = subpq.FrameQueueInit(&subtitleq, SUBPICTURE_QUEUE_SIZE, 0);
    if (ret < 0)
        return;
    ret = sampq.FrameQueueInit(&audioq, SAMPLE_QUEUE_SIZE, 1);
    if (ret < 0)
        return;
    // packet queue 暂时省略，由packetqueue构造函数完成
    videoq.PacketQueueInit();
    audioq.PacketQueueInit();
    subtitleq.PacketQueueInit();
    // clock init
    vidclk.ClockInit(&videoq.serial);
    audclk.ClockInit(&audioq.serial);
    extclk.ClockInit(&extclk.serial);

    audio_clock_serial = -1;
    if (startup_volume < 0)
        av_log(NULL, AV_LOG_WARNING, "-volume=%d < 0, setting to 0\n", startup_volume);
    if (startup_volume > 100)
        av_log(NULL, AV_LOG_WARNING, "-volume=%d > 100, setting to 100\n", startup_volume);
    startup_volume = av_clip(startup_volume, 0, 100);
    startup_volume = av_clip(SDL_MIX_MAXVOLUME * startup_volume / 100, 0, SDL_MIX_MAXVOLUME);
    audio_volume = startup_volume;
    muted = 0;
    av_sync_type = DEFAULTSYNCTYPE;
    read_tid = std::thread([this]()
                           { ReadThread(); });
}
int NewPlayer::ReadThread()
{
    ic = inFmtCtx;
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket *pkt = NULL;
    int64_t stream_start_time;
    int pkt_in_play_range = 0;
    AVDictionaryEntry *t;
    std::mutex wait_mutex;
    int scan_all_pmts_set = 0;
    int64_t pkt_ts;
    memset(st_index, -1, sizeof(st_index));

    eof = 0;
    pkt = AllocAVPacket();
    if (pkt == nullptr)
        return AVERROR(ENOMEM);
    if (genpts)
        ic->flags |= AVFMT_FLAG_GENPTS;

    av_format_inject_global_side_data(ic);

    if (ic->pb)
        ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

    if (seek_by_bytes < 0)
        seek_by_bytes = !!(ic->iformat->flags & AVFMT_TS_DISCONT) && strcmp("ogg", ic->iformat->name);

    max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
    if (start_time != AV_NOPTS_VALUE)
    {
        int64_t timestamp;

        timestamp = start_time;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE)
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",
                   filename, (double)timestamp / AV_TIME_BASE);
        }
    }

    realtime = is_realtime(ic);
    // for (unsigned int i = 0; i < ic->nb_streams; i++)
    //{
    //     AVStream *st = ic->streams[i];
    //     enum AVMediaType type = st->codecpar->codec_type;
    //     st->discard = AVDISCARD_ALL;
    //     if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1)
    //         /*if (avformat_match_stream_specifier(ic, st, wanted_stream_spec[type]) > 0)*/
    //             st_index[type] = i;
    // }
    st_index[AVMEDIA_TYPE_VIDEO] = 1;
    st_index[AVMEDIA_TYPE_AUDIO] = 1;
    for (i = 0; i < AVMEDIA_TYPE_NB; i++)
    {
        if (wanted_stream_spec[i] && st_index[i] == -1)
        {
            av_log(NULL, AV_LOG_ERROR, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i], av_get_media_type_string((AVMediaType)i));
            st_index[i] = INT_MAX;
        }
    }
    if (st_index[AVMEDIA_TYPE_VIDEO] > 0)
        st_index[AVMEDIA_TYPE_VIDEO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                -1, -1, NULL, 0);
    if (st_index[AVMEDIA_TYPE_AUDIO] > 0)
        st_index[AVMEDIA_TYPE_AUDIO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                -1,
                                st_index[AVMEDIA_TYPE_VIDEO],
                                NULL, 0);
    if (st_index[AVMEDIA_TYPE_SUBTITLE] > 0)
        st_index[AVMEDIA_TYPE_SUBTITLE] =
            av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
                                st_index[AVMEDIA_TYPE_SUBTITLE],
                                (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ? st_index[AVMEDIA_TYPE_AUDIO] : st_index[AVMEDIA_TYPE_VIDEO]),
                                NULL, 0);
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0)
    {
        stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
    }
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
    {
        stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
    }
    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0)
    {
        stream_component_open(st_index[AVMEDIA_TYPE_SUBTITLE]);
    }
    video_stream = st_index[AVMEDIA_TYPE_VIDEO];
    video_st = ic->streams[video_stream];
    audio_stream = st_index[AVMEDIA_TYPE_AUDIO];
    audio_st = ic->streams[audio_stream];
    if (video_stream < 0 && audio_stream < 0)
    {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n",
               filename);
        return -1;
    }
    if (infinite_buffer < 0 && realtime)
        infinite_buffer = 1;

    //开始读取packet
    while (true)
    {
        if (abort_request)
            break;
        if (paused != last_paused)
        {
            last_paused = paused;
            if (paused)
                read_pause_return = av_read_pause(ic);
            else
                av_read_play(ic);
        }

        if (seek_req)
        {
            int64_t seek_target = seek_pos;
            int64_t seek_min = seek_rel > 0 ? seek_target - seek_rel + 2 : INT64_MIN;
            int64_t seek_max = seek_rel < 0 ? seek_target - seek_rel - 2 : INT64_MAX;
            // FIXME the +-2 is due to rounding being not done in the correct direction in generation
            //      of the seek_pos/seek_rel variables

            ret = avformat_seek_file(ic, -1, seek_min, seek_target, seek_max, seek_flags);
            if (ret < 0)
            {
                av_log(NULL, AV_LOG_ERROR,
                       "%s: error while seeking\n", ic->url);
            }
            else
            {
                if (audio_stream >= 0)
                    audioq.Flush();
                if (subtitle_stream >= 0)
                    subtitleq.Flush();
                if (video_stream >= 0)
                    videoq.Flush();
                if (seek_flags & AVSEEK_FLAG_BYTE)
                {
                    extclk.SetClock(NAN, 0);
                }
                else
                {
                    extclk.SetClock(seek_target / (double)AV_TIME_BASE, 0);
                }
            }
            seek_req = 0;
            queue_attachments_req = 1;
            eof = 0;
            if (paused)
                step_to_next_frame();
        }
        if (queue_attachments_req)
        {
            if (video_st && video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)
            {
                if ((ret = av_packet_ref(pkt, &video_st->attached_pic)) < 0)
                    return ret;
                videoq.PacketQueuePut(pkt);
                pkt->stream_index = video_stream;
                videoq.PacketQueuePut(pkt);
            }
            queue_attachments_req = 0;
        }
        /* if the queue are full, no need to read more */
        bool sizeBool = audioq.size + videoq.size + subtitleq.size > MAX_QUEUE_SIZE;
        bool audioEnough = audioq.HasEnoughPackets(audio_st, audio_stream);
        bool videoEnough = videoq.HasEnoughPackets(video_st, video_stream);
        bool subtitleEnough = subtitleq.HasEnoughPackets(subtitle_st, subtitle_stream);
        bool enoughBool = audioEnough && videoEnough && subtitleEnough;
        if (infinite_buffer < 1 && (sizeBool || enoughBool))
        {
            /* wait 10 ms */
            std::unique_lock<std::mutex> lock(wait_mutex);
            continue_read_thread.wait_until(lock, std::chrono::system_clock::now() + std::chrono::milliseconds(10));
            continue;
        }

        if (!paused &&
            (!audio_st || (auddec.finished == audioq.serial && sampq.FrameQueueNbRemaining() == 0)) &&
            (!video_st || (viddec.finished == videoq.serial && pictq.FrameQueueNbRemaining() == 0)))
        {
            if (loop != 1 && (!loop || --loop))
            {
                stream_seek(start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
            }
            else if (autoexit)
            {
                ret = AVERROR_EOF;
                return ret;
            }
        }

        // read packet
        ret = av_read_frame(ic, pkt);
        if (ret < 0)
        {
            if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !eof)
            {
                if (video_stream >= 0)
                {
                    pkt->stream_index = video_stream;
                    videoq.PacketQueuePut(pkt);
                }
                if (audio_stream >= 0)
                {
                    pkt->stream_index = audio_stream;
                    audioq.PacketQueuePut(pkt);
                }
                if (subtitle_stream >= 0)
                {
                    pkt->stream_index = subtitle_stream;
                    subtitleq.PacketQueuePut(pkt);
                }
                eof = 1;
            }
            if (ic->pb && ic->pb->error)
            {
                if (autoexit)
                    return ret;
                else
                    break;
            }
            std::unique_lock<std::mutex> lock(wait_mutex);
            continue_read_thread.wait_until(lock, std::chrono::system_clock::now() + std::chrono::milliseconds(10));
            continue;
        }
        else
        {
            eof = 0;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */
        stream_start_time = ic->streams[pkt->stream_index]->start_time;
        pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
        pkt_in_play_range = duration == AV_NOPTS_VALUE ||
                            (pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) *
                                        av_q2d(ic->streams[pkt->stream_index]->time_base) -
                                    (double)(start_time != AV_NOPTS_VALUE ? start_time : 0) / 1000000 <=
                                ((double)duration / 1000000);
        if (pkt->stream_index == audio_stream && pkt_in_play_range)
        {
            audioq.PacketQueuePut(pkt);
        }
        else if (pkt->stream_index == video_stream && pkt_in_play_range && !(video_st->disposition & AV_DISPOSITION_ATTACHED_PIC))
        {
            videoq.PacketQueuePut(pkt);
        }
        else if (pkt->stream_index == subtitle_stream && pkt_in_play_range)
        {
            subtitleq.PacketQueuePut(pkt);
        }
        else
        {
            av_packet_unref(pkt);
        }
        // av_log_info("put a packet,audio queue duration:%d,video queue duration:%d\n", audioq.duration, videoq.duration);
    }
    av_log_info("de packet over,audio and video byte size:%d\n", audioq.size + videoq.size);
    return 0;
}
int NewPlayer::is_realtime(AVFormatContext *s)
{
    if (!strcmp(s->iformat->name, "rtp") || !strcmp(s->iformat->name, "rtsp") || !strcmp(s->iformat->name, "sdp"))
        return 1;

    if (s->pb && (!strncmp(s->url, "rtp:", 4) || !strncmp(s->url, "udp:", 4)))
        return 1;
    return 0;
}
void NewPlayer::stream_component_open(int stream_index)
{
    AVFormatContext *ic = inFmtCtx;
    AVCodecContext *avctx = nullptr;
    AVStream *stream = ic->streams[stream_index];
    const AVCodec *codec;
    const char *forced_codec_name = NULL;
    int sample_rate, nb_channels;
    int64_t channel_layout;
    int ret = 0;
    int stream_lowres = lowres;

    switch (stream->codecpar->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        avctx = deAudioCodecCtx;
        last_audio_stream = stream_index;
        forced_codec_name = ""; // todo 设置合适的codec name
        break;
    case AVMEDIA_TYPE_VIDEO:
        avctx = deVideoCodecCtx;
        last_video_stream = stream_index;
        forced_codec_name = ""; // todo 设置合适的codec name
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        // todo 暂时不支持字幕
        return;
    default:
        return;
    }
    codec = avcodec_find_decoder(avctx->codec_id);
    avctx->pkt_timebase = stream->time_base;
    if (stream_lowres > codec->max_lowres)
    {
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
               codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }
    avctx->lowres = stream_lowres;
    if (fast)
        avctx->flags2 |= AV_CODEC_FLAG2_FAST;

    eof = 0;
    // ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;//todo 慎重加此选项
    switch (stream->codecpar->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        audio_tgt.fmt = avctx->sample_fmt;
        audio_tgt.freq = avctx->sample_rate;
        /*audio_tgt.channel_layout = avctx->channel_layout;
         audio_tgt.channels = avctx->channels;*/
        audio_tgt.frame_size = av_samples_get_buffer_size(NULL, avctx->ch_layout.nb_channels, 1, audio_tgt.fmt, 1);
        audio_tgt.bytes_per_sec = av_samples_get_buffer_size(NULL, avctx->ch_layout.nb_channels, audio_tgt.freq, audio_tgt.fmt, 1);
        if (audio_tgt.bytes_per_sec <= 0 || audio_tgt.frame_size <= 0)
        {
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
            return;
        }
        audio_hw_buf_size = 4096 * 4; // todo 可能有错
        audio_src = audio_tgt;
        audio_buf_size = 0;
        audio_buf_index = 0;
        audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
        audio_diff_avg_count = 0;
        audio_diff_threshold = (double)(audio_hw_buf_size) / audio_tgt.bytes_per_sec;
        audio_stream = stream_index;
        audio_st = ic->streams[stream_index];
        // PacketQueue* queue = &audioq;
        ret = auddec.DecoderInit(avctx, &audioq, &continue_read_thread);
        if (ret < 0)
            return;
        if ((ic->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !ic->iformat->read_seek)
        {
            auddec.start_pts = audio_st->start_time;
            auddec.start_pts_tb = audio_st->time_base;
        }
        ret = auddec.DecoderStart(&audio_decoder_tid, 1);
        audio_decoder_tid = std::thread([this]()
                                        { audio_thread(); });

        break;
    case AVMEDIA_TYPE_VIDEO:
        video_stream = stream_index;
        video_st = ic->streams[stream_index];
        ret = viddec.DecoderInit(avctx, &videoq, &continue_read_thread);
        if (ret < 0)
            return;
        ret = viddec.DecoderStart(&video_decoder_tid, 0);
        video_decoder_tid = std::thread([this]()
                                        { video_thread(); });
        queue_attachments_req = 1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        // todo 暂时不支持字幕
        break;
    default:
        break;
    }
}
void NewPlayer::step_to_next_frame()
{
    if (paused)
        stream_toggle_pause();
    step = 1;
}
void NewPlayer::stream_toggle_pause()
{
    if (paused)
    {
        frame_timer += av_gettime_relative() / 1000000.0 - vidclk.last_updated;
        if (read_pause_return != AVERROR(ENOSYS))
        {
            vidclk.paused = 0;
        }
        vidclk.SetClock(vidclk.GetClock(), vidclk.serial);
    }
    extclk.SetClock(extclk.GetClock(), extclk.serial);
    paused = audclk.paused = vidclk.paused = extclk.paused = !paused;
}
void NewPlayer::stream_seek(int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!seek_req)
    {
        seek_pos = pos;
        seek_rel = rel;
        seek_flags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes)
            seek_flags |= AVSEEK_FLAG_BYTE;
        seek_req = 1;
        continue_read_thread.notify_all(); // todo all or one
    }
}

int NewPlayer::audio_thread()
{
    AVFrame *frame = av_frame_alloc();
    Frame *af;
    int got_frame = 0;
    AVRational tb;
    int ret = 0;
    if (!frame)
        return AVERROR(ENOMEM);

    while (!abort_request)
    {
        av_log_info("audio while start\n");
        if ((got_frame = decoder_decode_frame(&auddec, frame, NULL)) < 0)
            break;

        if (got_frame)
        {
            av_log_info("got a audio frame start\n");
            tb = {1, frame->sample_rate};
            if (!(af = sampq.frame_queue_peek_writable()))
                break;
            af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            af->pos = frame->pkt_pos;
            af->serial = auddec.pkt_serial;
            af->duration = av_q2d({frame->nb_samples, frame->sample_rate});
            av_frame_move_ref(af->frame, frame);
            sampq.FrameQueuePush();
            av_log_info("got a audio frame end\n");
        }
        av_log_info("audio while end\n");
    }
    // ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF||
    if (frame != nullptr)
        av_frame_free(&frame);
    av_log_info("audio decode frame end\n");
    return ret;
}
int NewPlayer::video_thread()
{
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = video_st->time_base;
    AVRational frame_rate = av_guess_frame_rate(ic, video_st, NULL);

    if (!frame)
        return AVERROR(ENOMEM);
    while (!abort_request)
    {
        ret = get_video_frame(frame);
        if (ret < 0)
            break;
        if (!ret)
            continue;
        duration = (frame_rate.num && frame_rate.den ? av_q2d({frame_rate.den, frame_rate.num}) : 0);
        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
        ret = queue_picture(frame, pts, duration, frame->pkt_pos, viddec.pkt_serial);
        av_frame_unref(frame);
        if (ret < 0)
            break;
    }
the_end:
    if (frame != nullptr)
        av_frame_free(&frame);
    av_log_info("video decoder frame end\n");
    return 0;
}
int NewPlayer::get_video_frame(AVFrame *frame)
{
    int got_picture;

    if ((got_picture = decoder_decode_frame(&viddec, frame, NULL)) < 0)
        return -1;

    if (got_picture)
    {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE)
            dpts = av_q2d(video_st->time_base) * frame->pts;

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(ic, video_st, frame);

        //|| (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER) todo 暂定视频同步音频,不考虑其他同步方法
        if (framedrop > 0)
        {
            if (frame->pts != AV_NOPTS_VALUE)
            {
                double diff = dpts - audclk.GetClock(); // todo 暂时指定audclk为master clock
                if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                    diff - frame_last_filter_delay < 0 &&
                    viddec.pkt_serial == vidclk.serial &&
                    videoq.nb_packets)
                {
                    frame_drops_early++;
                    av_frame_unref(frame);
                    got_picture = 0;
                }
            }
        }
    }

    return got_picture;
}
int NewPlayer::decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub)
{

    int ret = AVERROR(EAGAIN);

    for (;;)
    {
        if (d->queue->serial == d->pkt_serial)
        {
            do
            {
                if (d->queue->abort_request)
                    return -1;

                switch (d->avctx->codec_type)
                {
                case AVMEDIA_TYPE_VIDEO:
                    ret = avcodec_receive_frame(d->avctx, frame);
                    if (ret >= 0)
                    {
                        if (decoder_reorder_pts == -1)
                        {
                            frame->pts = frame->best_effort_timestamp;
                        }
                        else if (!decoder_reorder_pts)
                        {
                            frame->pts = frame->pkt_dts;
                        }
                    }
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    ret = avcodec_receive_frame(d->avctx, frame);
                    if (ret >= 0)
                    {
                        AVRational tb = {1, frame->sample_rate};
                        if (frame->pts != AV_NOPTS_VALUE)
                            frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
                        else if (d->next_pts != AV_NOPTS_VALUE)
                            frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                        if (frame->pts != AV_NOPTS_VALUE)
                        {
                            d->next_pts = frame->pts + frame->nb_samples;
                            d->next_pts_tb = tb;
                        }
                    }
                    break;
                default:
                    break;
                }
                if (ret == AVERROR_EOF)
                {
                    d->finished = d->pkt_serial;
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }
                if (ret >= 0)
                    return 1;
            } while (ret != AVERROR(EAGAIN));
        }

        do
        {
            if (d->queue->nb_packets == 0)
                d->empty_queue_cond->notify_all(); // todo all or one
            if (d->packet_pending)
            {
                d->packet_pending = 0;
            }
            else
            {
                int old_serial = d->pkt_serial;
                if (d->queue->PacketQueueGet(d->pkt, 1, &d->pkt_serial) < 0)
                    return -1;
                if (old_serial != d->pkt_serial)
                {
                    avcodec_flush_buffers(d->avctx);
                    d->finished = 0;
                    d->next_pts = d->start_pts;
                    d->next_pts_tb = d->start_pts_tb;
                }
            }
            if (d->queue->serial == d->pkt_serial)
                break;
            av_packet_unref(d->pkt);
        } while (1);

        if (d->avctx->codec_type == AVMEDIA_TYPE_SUBTITLE)
        {
            int got_frame = 0;
            ret = avcodec_decode_subtitle2(d->avctx, sub, &got_frame, d->pkt);
            if (ret < 0)
            {
                ret = AVERROR(EAGAIN);
            }
            else
            {
                if (got_frame && !d->pkt->data)
                {
                    d->packet_pending = 1;
                }
                ret = got_frame ? 0 : (d->pkt->data ? AVERROR(EAGAIN) : AVERROR_EOF);
            }
            av_packet_unref(d->pkt);
        }
        else
        {
            if (avcodec_send_packet(d->avctx, d->pkt) == AVERROR(EAGAIN))
            {
                av_log(d->avctx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                d->packet_pending = 1;
            }
            else
            {
                av_packet_unref(d->pkt);
            }
        }
    }
}
int NewPlayer::queue_picture(AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
    Frame *vp;

    if (!(vp = pictq.frame_queue_peek_writable()))
        return -1;

    vp->sar = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;

    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    vp->serial = serial;

    av_frame_move_ref(vp->frame, src_frame);
    pictq.FrameQueuePush();
    return 0;
}
void NewPlayer::video_refresh(double &remaining_time)
{
    double time;
    Frame *sp, *sp2;
    if (!paused && get_master_sync_type() == AV_SYNC_EXTERNAL_CLOCK && realtime)
        check_external_clock_speed();
    if (!display_disable && show_mode != SHOW_MODE_VIDEO && audio_st)
    {
        time = av_gettime_relative() / 1000000.0;
        if (force_refresh || last_vis_time + rdftspeed < time)
        {
            av_log_info("rdft display\n");
            video_display();
            last_vis_time = time;
        }
        remaining_time = FFMIN(remaining_time, last_vis_time + rdftspeed - time);
    }

    if (video_st)
    {
    retry:
        if (pictq.FrameQueueNbRemaining() == 0)
        {
            av_log_info("frame queue number consumed all,end\n");
        }
        else
        {
            double last_duration, duration, delay;
            Frame *vp, *lastvp;
            lastvp = pictq.frame_queue_peek_last();
            vp = pictq.frame_queue_peek();
            if (vp->serial != videoq.serial)
            {
                pictq.frame_queue_next();
                goto retry;
            }
            if (lastvp->serial != vp->serial)
                frame_timer = av_gettime_relative() / 1000000.0;
            if (paused)
                goto display;

            /* compute nominal last_duration */
            last_duration = vp_duration(lastvp, vp);
            delay = compute_target_delay(last_duration);

            time = av_gettime_relative() / 1000000.0;
            if (time < frame_timer + delay)
            {
                remaining_time = FFMIN(frame_timer + delay - time, remaining_time);
                goto display;
            }

            frame_timer += delay;
            if (delay > 0 && time - frame_timer > AV_SYNC_THRESHOLD_MAX)
                frame_timer = time;

            pictq.mtx.lock();
            if (!isnan(vp->pts))
                update_video_pts(vp->pts, vp->pos, vp->serial);
            pictq.mtx.unlock();

            if (pictq.FrameQueueNbRemaining() > 1)
            {
                Frame *nextvp = pictq.frame_queue_peek_next();
                duration = vp_duration(vp, nextvp);
                if (!step && (framedrop > 0 || (framedrop && get_master_sync_type() != AV_SYNC_VIDEO_MASTER)) && time > frame_timer + duration)
                {
                    frame_drops_late++;
                    pictq.frame_queue_next();
                    goto retry;
                }
            }
            pictq.frame_queue_next();
            force_refresh = 1;

            if (step && !paused)
                stream_toggle_pause();
        }
    display:
        /* display picture */
        if (!display_disable && force_refresh && show_mode == SHOW_MODE_VIDEO && pictq.rindex_shown)
            video_display();
    }
    force_refresh = 0;
    // show_status 暂时不支持
}
int NewPlayer::get_master_sync_type() const
{
    if (av_sync_type == AV_SYNC_VIDEO_MASTER)
    {
        if (video_st)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    }
    else if (av_sync_type == AV_SYNC_AUDIO_MASTER)
    {
        if (audio_st)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    }
    else
    {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}
void NewPlayer::check_external_clock_speed()
{
    if ((video_stream >= 0 && videoq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES) ||
        (audio_stream >= 0 && audioq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES))
    {
        extclk.SetClockSpeed(FFMAX(EXTERNAL_CLOCK_SPEED_MIN, extclk.speed - EXTERNAL_CLOCK_SPEED_STEP));
    }
    else if ((video_stream < 0 || videoq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) &&
             (audio_stream < 0 || audioq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES))
    {
        extclk.SetClockSpeed(FFMIN(EXTERNAL_CLOCK_SPEED_MAX, extclk.speed + EXTERNAL_CLOCK_SPEED_STEP));
    }
    else
    {
        double speed = extclk.speed;
        if (speed != 1.0)
            extclk.SetClockSpeed(speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
    }
}
void NewPlayer::video_display()
{
    if (audio_st && show_mode != SHOW_MODE_VIDEO)
        video_audio_display();
    else if (video_st)
        video_image_display();
}
void NewPlayer::video_audio_display()
{
}
void NewPlayer::video_image_display()
{
    Frame *vp;
    vp = pictq.frame_queue_peek_last();
    // todo 将数据传递给unity
    if (deVideoFrame == vp->frame)
        return;
    deVideoFrame = vp->frame;
    av_log_info("video frame pts:%d\n", deVideoFrame->pts);
    RescaleDevideoFrame();
    if (this->data == nullptr)
        return;
    memcpy(this->data, enVideoFrame->data[0], deVideoFrame->width * deVideoFrame->height * 4 * sizeof(uint8_t));
}
double NewPlayer::vp_duration(Frame *vp, Frame *nextvp) const
{
    if (vp->serial == nextvp->serial)
    {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0 || duration > max_frame_duration)
            return vp->duration;
        else
            return duration;
    }
    else
    {
        return 0.0;
    }
}
double NewPlayer::compute_target_delay(double delay)
{
    double sync_threshold, diff = 0;

    /* update delay to follow master synchronisation source */
    if (get_master_sync_type() != AV_SYNC_VIDEO_MASTER)
    {
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
        diff = vidclk.GetClock() - get_master_clock();

        /* skip or repeat frame. We take into account the
           delay to compute the threshold. I still don't know
           if it is the best guess */
        sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if (!isnan(diff) && fabs(diff) < max_frame_duration)
        {
            if (diff <= -sync_threshold)
                delay = FFMAX(0, delay + diff);
            else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
                delay = delay + diff;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }

    av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n",
           delay, -diff);

    return delay;
}
double NewPlayer::get_master_clock() const
{
    double val;

    switch (get_master_sync_type())
    {
    case AV_SYNC_VIDEO_MASTER:
        val = vidclk.GetClock();
        break;
    case AV_SYNC_AUDIO_MASTER:
        val = audclk.GetClock();
        break;
    default:
        val = extclk.GetClock();
        break;
    }
    return val;
}
void NewPlayer::update_video_pts(double pts, int64_t pos, int serial)
{
    vidclk.SetClock(pts, serial);
    extclk.sync_clock_to_slave(&vidclk);
}
void NewPlayer::repfresh_loop_wait_event()
{
    int frameNumber = 0;
    double remaining_time = 0.0;
    while (!abort_request)
    {
        if (remaining_time > 0.0)
            av_usleep((int64_t)(remaining_time * 1000000.0));
        remaining_time = REFRESH_RATE;
        if (show_mode != SHOW_MODE_NONE && (!paused || force_refresh))
        {
            video_refresh(remaining_time);
            frameNumber++;
        }
    }
    av_log_info("video loop end\n");
}
void NewPlayer::audiocallback_loop_wait_event()
{
    int frameNumber = 0;
    float *stream = new float[4096];
    double remaining_time = 0.0;
    while (!abort_request)
    {
        av_log_info("audio_callback is used\n");
        audio_callback((uint8_t *)stream, 4096 * sizeof(float));
        frameNumber++;
        // av_usleep(AV_TIME_BASE/100);
    }
    delete[] stream;
    av_log_info("audio callback end\n");
}
void NewPlayer::audio_callback(uint8_t *stream, int len)
{
    int audio_size, len1;

    audio_callback_time = av_gettime_relative();

    while (len > 0)
    {
        if (audio_buf_index >= audio_buf_size)
        {
            audio_size = audio_decode_frame();
            audio_size = audio_size / audioStream->codecpar->ch_layout.nb_channels; // todo 记得修改，临时性的代码
            if (audio_size < 0)
            {
                /* if error, just output silence */
                audio_buf = NULL;
                audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / audio_tgt.frame_size * audio_tgt.frame_size;
            }
            else
            {
                if (show_mode != SHOW_MODE_VIDEO)
                    update_sample_display((int16_t *)audio_buf, audio_size);
                audio_buf_size = audio_size;
            }
            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;
        if (len1 > len)
            len1 = len;
        if (!muted && audio_buf && audio_volume == SDL_MIX_MAXVOLUME)
            memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
        else
        {
            memset(stream, 0, len1);
            /*if (!is->muted && is->audio_buf)*/
            // todo 暂时保留混音操作 SDL_MixAudioFormat(stream, (uint8_t*)is->audio_buf + is->audio_buf_index, AUDIO_S16SYS, len1, is->audio_volume);
        }
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
    audio_write_buf_size = audio_buf_size - audio_buf_index;
    /* Let's assume the audio driver that is used by SDL has two periods. */
    if (!isnan(audio_clock))
    {
        audclk.SetClockAt(audio_clock - (double)(2 * audio_hw_buf_size + audio_write_buf_size) / audio_tgt.bytes_per_sec,
                          audio_clock_serial,
                          audio_callback_time / 1000000.0);
        extclk.sync_clock_to_slave(&audclk);
    }
}
int NewPlayer::audio_decode_frame()
{
    int data_size, resampled_data_size;
    int64_t dec_channel_layout;
    av_unused double audio_clock0;
    int wanted_nb_samples;
    Frame *af;

    if (paused)
        return -1;
    do
    {
#if defined(_WIN32)
        while (sampq.FrameQueueNbRemaining() == 0)
        {
            double time = 1000000LL * audio_hw_buf_size / audio_tgt.bytes_per_sec / 2;
            if ((av_gettime_relative() - audio_callback_time) > time)
                return -1;
            av_usleep(1000);
        }
#endif
        if (!(af = sampq.frame_queue_peek_readable()))
            return -1;
        sampq.frame_queue_next();
    } while (af->serial != audioq.serial);

    data_size = av_samples_get_buffer_size(NULL, af->frame->ch_layout.nb_channels,
                                           af->frame->nb_samples,
                                           (AVSampleFormat)af->frame->format, 1);

    dec_channel_layout = af->frame->ch_layout.order;
    wanted_nb_samples = synchronize_audio(af->frame->nb_samples);

    if (af->frame->format != audio_src.fmt ||
        dec_channel_layout != audio_src.channel_layout ||
        af->frame->sample_rate != audio_src.freq ||
        (wanted_nb_samples != af->frame->nb_samples && swr_ctx))
    {
        swr_free(&swr_ctx);
        // todo 获取重采样器
        /*swr_ctx = swr_alloc_set_opts(NULL,
            audio_tgt.channel_layout, audio_tgt.fmt, audio_tgt.freq,
            dec_channel_layout, (AVSampleFormat)af->frame->format, af->frame->sample_rate,
            0, NULL);
        if (!swr_ctx || swr_init(swr_ctx) < 0) {
            av_log(NULL, AV_LOG_ERROR,
                "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                af->frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat)af->frame->format), af->frame->ch_layout.nb_channels,
                audio_tgt.freq, av_get_sample_fmt_name(audio_tgt.fmt), audio_tgt.channels);
            swr_free(&swr_ctx);
            return -1;
        }*/
        audio_src.channel_layout = dec_channel_layout;
        audio_src.channels = af->frame->ch_layout.nb_channels;
        audio_src.freq = af->frame->sample_rate;
        audio_src.fmt = (AVSampleFormat)af->frame->format;
    }
    if (swr_ctx)
    {
        const uint8_t **in = (const uint8_t **)af->frame->extended_data;
        uint8_t **out = &audio_buf1;
        int out_count = (int64_t)wanted_nb_samples * audio_tgt.freq / af->frame->sample_rate + 256;
        int out_size = av_samples_get_buffer_size(NULL, audio_tgt.channels, out_count, audio_tgt.fmt, 0);
        int len2;
        if (out_size < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            return -1;
        }
        if (wanted_nb_samples != af->frame->nb_samples)
        {
            if (swr_set_compensation(swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * audio_tgt.freq / af->frame->sample_rate,
                                     wanted_nb_samples * audio_tgt.freq / af->frame->sample_rate) < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "swr_set_compensation() failed\n");
                return -1;
            }
        }
        av_fast_malloc(&audio_buf1, &audio_buf1_size, out_size);
        if (!audio_buf1)
            return AVERROR(ENOMEM);
        len2 = swr_convert(swr_ctx, out, out_count, in, af->frame->nb_samples);
        if (len2 < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            return -1;
        }
        if (len2 == out_count)
        {
            av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if (swr_init(swr_ctx) < 0)
                swr_free(&swr_ctx);
        }
        audio_buf = audio_buf1;
        resampled_data_size = len2 * audio_tgt.channels * av_get_bytes_per_sample(audio_tgt.fmt);
    }
    else
    {
        audio_buf = af->frame->data[0];
        av_log_info("callback a audio frame index:%d,pst:%d\n", callbackAudioCount++, af->frame->pts);
        resampled_data_size = data_size;
    }

    audio_clock0 = audio_clock;
    /* update the audio clock with the pts */
    if (!isnan(af->pts))
        audio_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
    else
        audio_clock = NAN;
    audio_clock_serial = af->serial;

    return resampled_data_size;
}
int NewPlayer::synchronize_audio(int nb_samples)
{
    int wanted_nb_samples = nb_samples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (get_master_sync_type() != AV_SYNC_AUDIO_MASTER)
    {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;

        diff = audclk.GetClock() - get_master_clock();

        if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD)
        {
            audio_diff_cum = diff + audio_diff_avg_coef * audio_diff_cum;
            if (audio_diff_avg_count < AUDIO_DIFF_AVG_NB)
            {
                /* not enough measures to have a correct estimate */
                audio_diff_avg_count++;
            }
            else
            {
                /* estimate the A-V difference */
                avg_diff = audio_diff_cum * (1.0 - audio_diff_avg_coef);

                if (fabs(avg_diff) >= audio_diff_threshold)
                {
                    wanted_nb_samples = nb_samples + (int)(diff * audio_src.freq);
                    min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }
                av_log(NULL, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",
                       diff, avg_diff, wanted_nb_samples - nb_samples,
                       audio_clock, audio_diff_threshold);
            }
        }
        else
        {
            /* too big difference : may be initial PTS errors, so
               reset A-V filter */
            audio_diff_avg_count = 0;
            audio_diff_cum = 0;
        }
    }

    return wanted_nb_samples;
}
void NewPlayer::update_sample_display(short *samples, int samples_size)
{
    int size, len;

    size = samples_size / sizeof(short);
    while (size > 0)
    {
        len = SAMPLE_ARRAY_SIZE - sample_array_index;
        if (len > size)
            len = size;
        memcpy(sample_array + sample_array_index, samples, len * sizeof(short));
        samples += len;
        sample_array_index += len;
        if (sample_array_index >= SAMPLE_ARRAY_SIZE)
            sample_array_index = 0;
        size -= len;
    }
}
void NewPlayer::close_player()
{
    abort_request = 1;
    read_tid.join();
    /* close each stream */
    if (audio_stream >= 0)
        stream_component_close(audio_stream);
    if (video_stream >= 0)
        stream_component_close(video_stream);
    if (subtitle_stream >= 0)
        stream_component_close(subtitle_stream);

    // avformat_close_input(&ic);
    videoq.destory();
    audioq.destory();
    subtitleq.destory();

    videoq.destory();
    audioq.destory();
    subpq.destory();
}
void NewPlayer::stream_component_close(int stream_index)
{
    AVCodecParameters *codecpar;
    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    codecpar = ic->streams[stream_index]->codecpar;

    switch (codecpar->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        auddec.DecoderAbort(&sampq);
        auddec.Destory();
        swr_free(&swr_ctx);
        av_freep(&audio_buf1);
        audio_buf1_size = 0;
        audio_buf = NULL;

        //暂时不支持rdft
        /*if (rdft) {
            av_rdft_end(rdft);
            av_freep(&rdft_data);
            rdft = NULL;
            rdft_bits = 0;
        }*/
        break;
    case AVMEDIA_TYPE_VIDEO:
        viddec.DecoderAbort(&pictq);
        viddec.Destory();
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        subdec.DecoderAbort(&subpq);
        subdec.Destory();
        break;
    default:
        break;
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    switch (codecpar->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        audio_st = NULL;
        audio_stream = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        video_st = NULL;
        video_stream = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        subtitle_st = NULL;
        subtitle_stream = -1;
        break;
    default:
        break;
    }
}
bool NewPlayer::CopyAudioDataToUnity(void *data, int length)
{

    if (paused)
    {
        stream_toggle_pause();
    }
    audio_callback((uint8_t *)data, length);
    if (callbackAudioCount > audioStream->nb_frames)
    {
        stream_toggle_pause();
        /*step = 0;*/
        callbackAudioCount = 1;
        return false;
    }
    return true;
}
bool NewPlayer::IsPaused() const
{
    return paused;
}
void NewPlayer::SetPaused(bool paused)
{
    this->paused = paused;
}
bool NewPlayer::CopyVideoDataToUnity(void *data)
{
    if (paused)
        return false;
    if (remaining_time > 0.0)
        av_usleep((int64_t)(remaining_time * 1000000.0));
    remaining_time = REFRESH_RATE;
    if (show_mode != SHOW_MODE_NONE && (!paused || force_refresh))
    {
        this->data = data;
        video_refresh(remaining_time);
        return true;
    }
    return false;
}
NewPlayer::NewPlayer(const char *srcFilePath) : FileContextBase(srcFilePath),
                                                AVFileContext(AV_CODEC_ID_AAC, AV_PIX_FMT_YUV420P, AV_CODEC_ID_GIF,
                                                              inFmtCtx->streams[getVideoStreamIndex()]->codecpar->width,
                                                              inFmtCtx->streams[getVideoStreamIndex()]->codecpar->height,
                                                              30,
                                                              inFmtCtx->streams[getVideoStreamIndex()]->codecpar->width,
                                                              inFmtCtx->streams[getVideoStreamIndex()]->codecpar->height),
                                                paused(false), abort_request(false), filename(const_cast<char *>(srcFilePath)),
                                                seekComponent(srcFilePath)
{
    sample_array = new int16_t[SAMPLE_ARRAY_SIZE];
    videoStream = inFmtCtx->streams[getVideoStreamIndex()];
    audioStream = inFmtCtx->streams[getAudioStreamIndex()];
    deVideoCodecCtx = OpenDecodecContextByStream(videoStream);
    AudioFileContext::VariableCheck(srcFilePath);
    VideoFileContext::VariableCheck(srcFilePath);
    av_frame_free(&deVideoFrame);
    swsCtx = InitSwsContext(deVideoCodecCtx, enVideoCodecCtx);
    // swrCtx = InitSwrContext(deAudioCodecCtx, enAudioCodecCtx);
}

void NewPlayer::GetInformation()
{
    InitData();
    while (audio_tgt.bytes_per_sec == 0)
        ;
    audio_loop_thid = std::thread([this]()
                                  { audiocallback_loop_wait_event(); });
    refresh_loop_thid = std::thread([this]()
                                    { repfresh_loop_wait_event(); });
    refresh_loop_thid.join();
    audio_loop_thid.join();

    // DecodePakcet();
    /*std::thread packetThread([this]() {ReadPacket(); });
    std::thread audioThread([this]() {ReceiveAudioFrame(); });
    std::thread videoThread([this]() {ReceiveVideoFrame(); });
    packetThread.join();
    audioThread.join();
    videoThread.join();*/
    /*av_log_info("audio packet count:%d\n",audioPacketCount);
    av_log_info("video packet count:%d\n",videoPacketCount);
    av_log_info("audio frame count:%d\n",audioFrameCount);*/
}

int NewPlayer::GetAudioSampleRate() const
{
    return audioStream->codecpar->sample_rate;
}

int NewPlayer::GetAudioSeconds() const
{
    return int((double)audioStream->duration / (double)audioStream->codecpar->sample_rate);
}

void NewPlayer::SeekFrameByPercent(float percent, void *data, int length)
{
    seekComponent.GetFrameDataByPercent(percent, data, length);
}

int NewPlayer::GetVideoWidth() const
{
    return videoStream->codecpar->width;
}

int NewPlayer::GetVideoHeight() const
{
    return videoStream->codecpar->height;
}

NewPlayer::~NewPlayer()
{
    abort_request = 1;
    close_player();
    video_decoder_tid.join();
    audio_decoder_tid.join();
    av_log_info("player end\n");
    delete sample_array;
}
