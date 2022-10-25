#include "RecordGif.h"

RecordGif::RecordGif(const char* dstFilepath, AVPixelFormat dePixfmt, int fps, float bitRatePercent, int width, int height):ret(0),isFirstFrame(true)
{
	enVideoCont = new EnCodecVideoContext(AV_CODEC_ID_GIF, width, height, fps, bitRatePercent);
	if (enVideoCont->GetResult())goto end;
	av_log_info("EnCodecVideoContext initialize success\n");
	swsCont = new AVSwsContext(dePixfmt, width, height, DEFAULTGIFINPUTPIXFMT,width,height);
	if (swsCont->GetResult()) goto end;
	av_log_info("AVSwsContext initialize success\n");
	filterCont = new FilterContext(enVideoCont->GetAVCodecContext());
	if (filterCont->GetResult() < 0) goto end;
	av_log_info("FilterContext initialize success\n");
	outfmtCont = new OutFormatContext(dstFilepath, {
		{enVideoCont->GetAVCodecContext(),videoStream} });
	if (outfmtCont->GetResult() < 0) goto end;
	av_log_info("OutFormatContext initialize success\n");
	deVideoFrame = AllocAVFrame();
	if (deVideoFrame == nullptr) goto end;
	av_log_info("de video frame alloc success\n");
	deVideoFrame->width = width;
	deVideoFrame->height = height;
	deVideoFrame->linesize[0] = width * 4;

	pts = 0;
	return;
end:
	ret = -1;
	return;
}

int RecordGif::GetResult() const
{
    return ret;
}

bool RecordGif::WriteGIFPreparition()
{
	return outfmtCont->WriteTofilePreparition();
}

bool RecordGif::WriteVideoToFile(void* data, int length)
{
	if (isFirstFrame) {
		isFirstFrame = false;
		return false;
	}
	AVFrame* sinkFrame = nullptr;
	FlipImage((unsigned char *)data, enVideoCont->GetAVCodecContext()->width, enVideoCont->GetAVCodecContext()->height);
	deVideoFrame->data[0] = (uint8_t*)data;
	AVFrame* swsFrame = CreateVideoFrame(DEFAULTGIFINPUTPIXFMT,enVideoCont->GetAVCodecContext()->width,enVideoCont->GetAVCodecContext()->height);
	if (swsCont->RescaleVideoFrame(deVideoFrame, swsFrame) == false)goto end;
	swsFrame->pts = pts++;
	if (filterCont->AddFrame(swsFrame) == false) goto end;
	sinkFrame = filterCont->GetFrame();
	if (sinkFrame == nullptr) goto end;

	return enVideoCont->EncodeFrame(*outfmtCont,videoStream,sinkFrame);
end:
	av_frame_free(&swsFrame);
	return false;
}

bool RecordGif::FlushEnVideoCodecBuffer()
{
	AVFrame* sinkFrame = nullptr;
	if (filterCont->FlushBuffer() == false)return false;
	do {
		sinkFrame = filterCont->GetFrame();
		enVideoCont->EncodeFrame(*outfmtCont, videoStream, sinkFrame);
	} while (sinkFrame != nullptr);
	return true;
}

bool RecordGif::WriteGIFTailer()
{
	return outfmtCont->WriteTofileClosure();
}

RecordGif::~RecordGif()
{
	delete swsCont;
	swsCont = nullptr;
	delete enVideoCont;
	enVideoCont = nullptr;
	delete outfmtCont;
	outfmtCont = nullptr;
	delete filterCont;
	filterCont = nullptr;
}
