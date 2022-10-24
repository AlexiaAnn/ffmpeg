#include "RecordMp4.h"

RecordMp4::RecordMp4(const char* dstFilepath, 
	int sampleRate, AVSampleFormat sampleFmt, AVChannelLayout chLayout, 
	AVPixelFormat dePixfmt, int fps, float bitRatePercent, int width, int height):ret(0),audioStream(nullptr),videoStream(nullptr)
{
	enAudioCont = new EnCodecAudioContext(DEFAULTAUDIOCODECID);
	if (enAudioCont->GetResult()<0) goto end;
	swrCont = new AVSwrContext(sampleRate, sampleFmt, chLayout,enAudioCont->GetAVCodecContext());
	if (swrCont->GetResult()<0) goto end;
	enVideoCont = new EnCodecVideoContext(DEFAULTVIDEOCODECID,width,height,fps,bitRatePercent);
	if (enVideoCont->GetResult())goto end;
	swsCont = new AVSwsContext(dePixfmt,width,height,enVideoCont->GetAVCodecContext());
	if (swsCont->GetResult()) goto end;
	outfmtCont = new OutFormatContext(dstFilepath, { 
		{enAudioCont->GetAVCodecContext(),audioStream},
		{enVideoCont->GetAVCodecContext(),videoStream} });

	deVideoFrame = AllocAVFrame();
	if (deVideoFrame == nullptr) goto end;
	deVideoFrame->width = width;
	deVideoFrame->height = height;
	deVideoFrame->linesize[0] = width * 4;

	deAudioFrame = AllocAVFrame();
	if (deAudioFrame == nullptr)goto end;
	deAudioFrame->sample_rate = sampleRate;
	deAudioFrame->format = sampleFmt;
	deAudioFrame->ch_layout = chLayout;

	return;

end:
	ret = -1;
	return;
}

bool RecordMp4::WriteAVPreparition()
{
	return outfmtCont->WriteTofilePreparition();
}

bool RecordMp4::WriteVideoToFile(void* data, int length)
{
	deVideoFrame->data[0] = (uint8_t*)data;
	swsCont->RescaleVideoFrame(deVideoFrame,enVideoCont->GetEncodecFrame());
	return enVideoCont->EncodeVideoFrame(outfmtCont->GetFormatContext(),videoStream);
}

bool RecordMp4::WriteAudioToFile(void* data, int length)
{
	deAudioFrame->data[0] = (uint8_t*)data;
	deAudioFrame->data[1] = (uint8_t*)((float*)data + length);
	deAudioFrame->nb_samples = length;
	swrCont->ResampleAudioFrame(deAudioFrame,*enAudioCont);
	return enAudioCont->EncodeAudioFrame(outfmtCont->GetFormatContext(),videoStream);
	
}

bool RecordMp4::FlushEnVideoCodecBuffer()
{
	return enVideoCont->FlushBuffer(outfmtCont->GetFormatContext(),videoStream);
}

bool RecordMp4::FlushEnAudioCodecBuffer()
{
	return enAudioCont->FlushBuffer(outfmtCont->GetFormatContext(),audioStream);
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
