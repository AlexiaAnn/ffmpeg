#pragma once
#include "AVFileContext.h"
#include "Clock.h"
#include "Frame.h"
#include "PacketQueue.h"
#include "FrameQueue.h"
#include "Decoder.h"
#include "PlayerMisco.h"
#include "SeekComponent.h"
#include <thread>
#include <libavcodec/avfft.h>

class NewPlayer : public AVFileContext
{
private:
	std::thread read_tid;
	AVInputFormat *iformat;
	int abort_request=0;
	int force_refresh;
	int paused;
	int last_paused;
	int queue_attachments_req;
	int seek_req;
	int seek_flags;
	int64_t seek_pos;
	int64_t seek_rel;
	int read_pause_return;
	AVFormatContext *ic;
	int realtime;

	Clock audclk;
	Clock vidclk;
	Clock extclk;

	FrameQueue pictq;
	FrameQueue subpq;
	FrameQueue sampq;

	Decoder auddec;
	Decoder viddec;
	Decoder subdec;

	int audio_stream;

	int av_sync_type;

	double audio_clock;
	int audio_clock_serial;
	double audio_diff_cum; /* used for AV difference average computation */
	double audio_diff_avg_coef;
	double audio_diff_threshold;
	int audio_diff_avg_count;
	AVStream *audio_st;
	PacketQueue audioq;
	int audio_hw_buf_size;
	uint8_t *audio_buf;
	uint8_t *audio_buf1;
	unsigned int audio_buf_size; /* in bytes */
	unsigned int audio_buf1_size;
	int audio_buf_index; /* in bytes */
	int audio_write_buf_size;
	int audio_volume;
	int muted;
	struct AudioParams audio_src;

	struct AudioParams audio_tgt;
	struct SwrContext *swr_ctx;
	int frame_drops_early;
	int frame_drops_late;

	enum ShowMode
	{
		SHOW_MODE_NONE = -1,
		SHOW_MODE_VIDEO = 0,
		SHOW_MODE_WAVES,
		SHOW_MODE_RDFT,
		SHOW_MODE_NB
	};
	int16_t *sample_array;
	int sample_array_index;

	int lowres = 0;
	int last_i_start;
	RDFTContext* rdft;
	int rdft_bits;
	FFTSample* rdft_data;
	int xpos;
	double last_vis_time;

	int subtitle_stream;
	AVStream *subtitle_st;
	PacketQueue subtitleq;

	double frame_timer;
	double frame_last_returned_time;
	double frame_last_filter_delay;
	int video_stream;
	AVStream *video_st;
	PacketQueue videoq;
	double max_frame_duration; // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
	struct SwsContext *img_convert_ctx;
	struct SwsContext *sub_convert_ctx;
	int eof;

	char *filename;
	int width, height, xleft, ytop;
	int step;

	int last_video_stream, last_audio_stream, last_subtitle_stream;

	std::condition_variable continue_read_thread;

	int startup_volume = 100;
	const char *wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
	int seek_by_bytes = -1;
	int64_t start_time = AV_NOPTS_VALUE;
	int64_t duration = AV_NOPTS_VALUE;
	int fast = 0;
	int genpts = 0;
	int autoexit = 0;
	int framedrop = -1;
	int loop = 10000;
	int infinite_buffer = -1;
	int display_disable = 0;
	double rdftspeed = 0.02;
	ShowMode show_mode = SHOW_MODE_VIDEO;
	std::thread refresh_loop_thid;
	std::thread audio_loop_thid;
	int64_t audio_callback_time;

	std::thread audio_decoder_tid;
	std::thread video_decoder_tid;

	int decoder_reorder_pts = -1;
	AVStream *videoStream;
	AVStream *audioStream;
	//音频pts控制
	int64_t audioNextPts;
	AVRational audioNextPtsTb;

	int callbackAudioCount = 1;
	int refreshVideoCount = 1;
	double remaining_time = 0.0;
	void* data;//指向unitybyte[]数据区
	SeekComponent seekComponent;
protected:
	/*void InitReadPacketThreadData();
	
	void ReadPacket();
	int PutPacketToQueue(std::mutex &mtx, std::queue<AVPacket *> &packetQueue, AVPacket *packet, int64_t &queueByteSize, int64_t &queueDuration);
	void VideoThread();
	void AudioThread();
	void ReceiveAudioFrame();
	int ReceiveVideoFrame(AVFrame *frame);
	int DecoderFrame(AVCodecContext *codecCtx, AVFrame *frame, std::queue<AVPacket *> &avPacketQueue, std::mutex &mtx,
					 int64_t &byteSize, int64_t &duration);
	int audio_open(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams* audio_hw_params);
	int PutVideoFrameToQueue(AVFrame *srcFrame, double pts, double duration);*/
	
	int ReadThread();
	int is_realtime(AVFormatContext *s);
	void stream_component_open(int stream_index);
	void step_to_next_frame();
	void stream_toggle_pause();
	void stream_seek(int64_t pos, int64_t rel, int seek_by_bytes);
	int audio_thread();
	int video_thread();
	int get_video_frame(AVFrame* frame);
	int decoder_decode_frame(Decoder* d, AVFrame* frame, AVSubtitle* sub);
	int queue_picture(AVFrame* src_frame, double pts, double duration, int64_t pos, int serial);
	void video_refresh(double& remaining_time);
	int get_master_sync_type() const;
	void check_external_clock_speed();
	void video_display();
	void video_audio_display();
	void video_image_display();
	double vp_duration(Frame* vp, Frame* nextvp) const;
	double compute_target_delay(double delay);
	double get_master_clock() const;
	void update_video_pts(double pts,int64_t pos,int serial);
	void repfresh_loop_wait_event();
	void audiocallback_loop_wait_event();
	void audio_callback(uint8_t* stream, int len);
	int audio_decode_frame();
	int synchronize_audio(int nb_samples);
	void update_sample_display(short* samples, int samples_size);
	void close_player();
	void stream_component_close(int stream_index);
	
public:
	NewPlayer(const char *srcFilePath);
	bool CopyVideoDataToUnity(void* data);
	void InitData();
	bool CopyAudioDataToUnity(void* data, int length);
	bool IsPaused() const;
	void SetPaused(bool paused);
	void GetInformation();
	int GetAudioSampleRate() const;
	int GetAudioSeconds() const;
	void SeekFrameByPercent(float percent,void* data,int length);
	int GetVideoWidth() const;
	int GetVideoHeight() const;
	~NewPlayer();
};