#include "RecordMp4Thread.h"

RecordMp4Thread::RecordMp4Thread(const char* dstFilepath,
	int sampleRate, AVSampleFormat sampleFmt, AVChannelLayout chLayout,
	AVPixelFormat dePixfmt, int fps, float bitRatePercent,
	int width, int height, int crfMin, int crfMax, int presetLevel) : ret(0), audioStream(nullptr), videoStream(nullptr), outfmtCont(nullptr),
	enAudioCont(nullptr), swrCont(nullptr), enVideoCont(nullptr),
	swsCont(nullptr), deAudioFrame(nullptr)
{
	enAudioCont = new EnCodecAudioContext(AV_CODEC_ID_AAC);
	if (enAudioCont->GetResult() < 0)
		goto end;
	swrCont = new AVSwrContext(sampleRate, sampleFmt, chLayout, enAudioCont->GetAVCodecContext());
	if (swrCont->GetResult() < 0)
		goto end;


#ifdef WINDOWS
	enVideoCont = new VideoEncoderThread(EncodeName::LIBX264, width, height, fps, bitRatePercent, crfMin, crfMax, presetLevel);
#endif
#ifdef ANDROID
		av_log_info("encode video code:hard");
		enVideoCont = new VideoEncoderThread(EncodeName::H264HLMEDIACODEC, width, height, fps, bitRatePercent, crfMin, crfMax, presetLevel);
#endif
	if (enVideoCont->GetResult() < 0)
		goto end;
#ifdef WINDOWS
	swsCont = new AVSwsContext(dePixfmt, width, height, enVideoCont->GetAVCodecContext());
#endif
#ifdef ANDROID
	av_log_info("LibyuvSwsContext");
	swsCont = new LibyuvSwsContext();
#endif
	if (swsCont->GetResult() < 0)
		goto end;

	outfmtCont = new OutFormatContext(dstFilepath, { {enAudioCont->GetAVCodecContext(), audioStream},
											{enVideoCont->GetAVCodecContext(), videoStream} });
	deAudioFrame = AllocAVFrame();
	if (deAudioFrame == nullptr)
		goto end;
	deAudioFrame->sample_rate = sampleRate;
	deAudioFrame->format = sampleFmt;
	deAudioFrame->ch_layout = chLayout;

	iFrameCount = 0;
	ret = 0;
	

	((VideoEncoderThread*)enVideoCont)->ThreadStart(outfmtCont,videoStream);
	return;

end:
	ret = -1;
	return;
}

bool RecordMp4Thread::WriteAVPreparition()
{
	return outfmtCont->WriteTofilePreparition();
}

bool RecordMp4Thread::WriteVideoToFile(void* data, int id)
{
	av_log_info("video frame id:%d",id);
	
	AVCodecContext* context = enVideoCont->GetAVCodecContext();

	AVFrame* frame = av_frame_alloc();
	frame->linesize[0] = context->width * 4;
	frame->width = context->width;
	frame->height = context->height;
	frame->data[0] = (uint8_t*)data;

	AVFrame* enVideoFrame = CreateVideoFrame(context->pix_fmt, context->width, context->height);
	if (enVideoFrame == nullptr) {
		av_log_error("alloc envideo frame failed,cant to rescale");
		return false;
	}
	swsCont->RescaleVideoFrame(frame, enVideoFrame);
	frame->data[0] = nullptr;
	av_frame_free(&frame);
	
	((VideoEncoderThread*)enVideoCont)->EncodeFrame(id,enVideoFrame);

	return true;
}

bool RecordMp4Thread::WriteAudioToFile(void* data, int length)
{
	if (deAudioFrame == nullptr)
		return false;
	deAudioFrame->data[0] = (uint8_t*)data;
	deAudioFrame->data[1] = (uint8_t*)((float*)data + length);
	deAudioFrame->nb_samples = length;
	swrCont->ResampleAudioFrame(deAudioFrame, *enAudioCont);
	return enAudioCont->EncodeFrame(*outfmtCont, audioStream);
}

// video frame duration end
// video frame duration
// mediacodec
// hlmediacodec

bool RecordMp4Thread::FlushEnVideoCodecBuffer()
{
	/*av_log_info("video frame duration end:[flip avg:%f,%f%],[rescale avg:%f,%f%],[encode avg:%f,%f%],[all avg:%f],[all:%f],[framecount:%d]",
		flipDurationAvg / iFrameCount, flipDurationAvg / allDurationAvg * 100,
		rescaleDurationAvg / iFrameCount, rescaleDurationAvg / allDurationAvg * 100,
		encodeDurationAvg / iFrameCount, encodeDurationAvg / allDurationAvg * 100,
		allDurationAvg / iFrameCount, allDurationAvg, iFrameCount);*/
	bool result = enVideoCont->FlushBuffer(*outfmtCont, videoStream);
	swsCont->GetTimeInfo();
	enVideoCont->GetTimeInfo();
	
	return result;
}

bool RecordMp4Thread::FlushEnAudioCodecBuffer()
{
	return enAudioCont->FlushBuffer(*outfmtCont, audioStream);
}

bool RecordMp4Thread::WriteAVTailer()
{
	return outfmtCont->WriteTofileClosure();
}

int RecordMp4Thread::GetResult() const
{
	return ret;
}

RecordMp4Thread::~RecordMp4Thread()
{
	av_log_info("%s start",__FUNCTION__);
	delete swsCont;
	swsCont = nullptr;
	delete swrCont;
	swrCont = nullptr;
	delete enAudioCont;
	enAudioCont = nullptr;
	delete enVideoCont;
	enVideoCont = nullptr;
	delete outfmtCont;
	outfmtCont = nullptr;
	av_log_info("%s end", __FUNCTION__);
}
