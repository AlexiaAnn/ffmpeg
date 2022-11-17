#include "RecordMp4.h"

RecordMp4::RecordMp4(const char *dstFilepath,
					 int sampleRate, AVSampleFormat sampleFmt, AVChannelLayout chLayout,
					 AVPixelFormat dePixfmt, int fps, float bitRatePercent,
					 int width, int height, int crfMin, int crfMax, int presetLevel) : ret(0), audioStream(nullptr), videoStream(nullptr), outfmtCont(nullptr),
																					   enAudioCont(nullptr), swrCont(nullptr), enVideoCont(nullptr),
																					   swsCont(nullptr), deVideoFrame(nullptr), deAudioFrame(nullptr)
{
	enAudioCont = new EnCodecAudioContext(DEFAULTAUDIOCODECID);
	if (enAudioCont->GetResult() < 0)
		goto end;
	swrCont = new AVSwrContext(sampleRate, sampleFmt, chLayout, enAudioCont->GetAVCodecContext());
	if (swrCont->GetResult() < 0)
		goto end;
#ifdef WINDOWS
	enVideoCont = new EnCodecVideoContext(EncodeName::LIBX264, width, height, fps, bitRatePercent, crfMin, crfMax, presetLevel);
#endif
#ifdef ANDROID
	av_log_info("encode video code:hard");
	enVideoCont = new EnCodecVideoContext(EncodeName::H264HLMEDIACODEC, width, height, fps, bitRatePercent, crfMin, crfMax, presetLevel);
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
	outfmtCont = new OutFormatContext(dstFilepath, {{enAudioCont->GetAVCodecContext(), audioStream},
													{enVideoCont->GetAVCodecContext(), videoStream}});

	deVideoFrame = AllocAVFrame();
	if (deVideoFrame == nullptr)
		goto end;
	deVideoFrame->width = width;
	deVideoFrame->height = height;
	deVideoFrame->linesize[0] = width * 4;

	deAudioFrame = AllocAVFrame();
	if (deAudioFrame == nullptr)
		goto end;
	deAudioFrame->sample_rate = sampleRate;
	deAudioFrame->format = sampleFmt;
	deAudioFrame->ch_layout = chLayout;

	iFrameCount = 0;
	ret = 0;
	return;

end:
	ret = -1;
	return;
}

bool RecordMp4::WriteAVPreparition()
{
	return outfmtCont->WriteTofilePreparition();
}

bool RecordMp4::WriteVideoToFile(void *data, int length)
{
	if (deVideoFrame == nullptr)
		return false;

	//测试屏蔽转码后的速度
	// if (iFrameCount == 0)
	// {
	// 	av_log_info("first frame rescale,test...");
	// 	iFrameCount++;
	// 	deVideoFrame->data[0] = (uint8_t *)data;
	// 	swsCont->RescaleVideoFrame(deVideoFrame, *enVideoCont);
	// 	return true;
	// }
	// else
	// {
	// 	iFrameCount++;
	// 	start = clock();
	// 	bool result = enVideoCont->EncodeFrame(*outfmtCont, videoStream);
	// 	end = clock();
	// 	float encodeDuration = float(end - start) / CLOCKS_PER_SEC;
	// 	encodeDurationAvg += encodeDuration;
	// 	allDurationAvg += encodeDuration;
	// 	return result;
	// }

	iFrameCount++;
	float duration = 0;

	// start = clock();
	// FlipImage((unsigned char *)data, enVideoCont->GetAVCodecContext()->width, enVideoCont->GetAVCodecContext()->height);
	// end = clock();
	// duration = (float)(end - start) / CLOCKS_PER_SEC;
	// flipDurationAvg += duration;
	// allDurationAvg += duration;

	deVideoFrame->data[0] = (uint8_t *)data;

	start = clock();
	swsCont->RescaleVideoFrame(deVideoFrame, *enVideoCont);
	end = clock();
	float rescaleDuration = float(end - start) / CLOCKS_PER_SEC;
	rescaleDurationAvg += rescaleDuration;
	allDurationAvg += rescaleDuration;

	start = clock();
	bool result = enVideoCont->EncodeFrame(*outfmtCont, videoStream);
	end = clock();
	float encodeDuration = float(end - start) / CLOCKS_PER_SEC;
	encodeDurationAvg += encodeDuration;
	allDurationAvg += encodeDuration;
	// av_log_info("video frame duration:[flip:%f],[rescale:%f],[encode:%f],frame index:%d",
	// 			duration, rescaleDuration, encodeDuration, iFrameCount);
	return result;
}

bool RecordMp4::WriteAudioToFile(void *data, int length)
{
	if (deAudioFrame == nullptr)
		return false;
	deAudioFrame->data[0] = (uint8_t *)data;
	deAudioFrame->data[1] = (uint8_t *)((float *)data + length);
	deAudioFrame->nb_samples = length;
	swrCont->ResampleAudioFrame(deAudioFrame, *enAudioCont);
	return enAudioCont->EncodeFrame(*outfmtCont, audioStream);
}

// video frame duration end
// video frame duration
// mediacodec
// hlmediacodec

bool RecordMp4::FlushEnVideoCodecBuffer()
{
	av_log_info("video frame duration end:[flip avg:%f,%f%],[rescale avg:%f,%f%],[encode avg:%f,%f%],[all avg:%f],[all:%f],[framecount:%d]",
				flipDurationAvg / iFrameCount, flipDurationAvg / allDurationAvg * 100,
				rescaleDurationAvg / iFrameCount, rescaleDurationAvg / allDurationAvg * 100,
				encodeDurationAvg / iFrameCount, encodeDurationAvg / allDurationAvg * 100,
				allDurationAvg / iFrameCount, allDurationAvg, iFrameCount);
	return enVideoCont->FlushBuffer(*outfmtCont, videoStream);
}

bool RecordMp4::FlushEnAudioCodecBuffer()
{
	return enAudioCont->FlushBuffer(*outfmtCont, audioStream);
}

bool RecordMp4::WriteAVTailer()
{
	return outfmtCont->WriteTofileClosure();
}

int RecordMp4::GetResult() const
{
	return ret;
}

RecordMp4::~RecordMp4()
{
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
}
