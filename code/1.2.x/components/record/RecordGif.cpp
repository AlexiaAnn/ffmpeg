#include "RecordGif.h"

RecordGif::RecordGif(const char* dstFilepath, AVPixelFormat dePixfmt, int fps,
	float bitRatePercent, int width, int height,
	int presetLevel) : ret(0), isFirstFrame(true), inFrameCount(0),mTempData(nullptr)
{
	enVideoCont = new EnCodecVideoContext(AV_CODEC_ID_GIF, width, height, fps, bitRatePercent, DEFAULTCRFMIN, DEFAULTCRFMAX, presetLevel);
	if (enVideoCont->GetResult())
		goto end;
	av_log_info("EnCodecVideoContext initialize success\n");
	swsCont = new AVSwsContext(dePixfmt, width, height, DEFAULTGIFINPUTPIXFMT, width, height);
	if (swsCont->GetResult())
		goto end;
	av_log_info("AVSwsContext initialize success\n");
	filterCont = new FilterContext(enVideoCont->GetAVCodecContext());
	if (filterCont->GetResult() < 0)
		goto end;
	av_log_info("FilterContext initialize success\n");
	outfmtCont = new OutFormatContext(dstFilepath, { {enVideoCont->GetAVCodecContext(), videoStream} });
	if (outfmtCont->GetResult() < 0)
		goto end;
	av_log_info("OutFormatContext initialize success\n");
	deVideoFrame = AllocAVFrame();
	if (deVideoFrame == nullptr)
		goto end;
	av_log_info("de video frame alloc success\n");
	deVideoFrame->width = width;
	deVideoFrame->height = height;
	deVideoFrame->linesize[0] = width * 4;

	mTempData = (uint8_t*)malloc(sizeof(char) * width * height * 4);

	pts = 0;
	ret = 0;
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
	++inFrameCount;
	//if (isFirstFrame)
	//{
	//	isFirstFrame = false;
	//	return false;
	//}
	if (deVideoFrame == nullptr)
		return false;

	bool result = false;
	AVFrame* sinkFrame = nullptr;
	//flip image
	FlipImage((uint8_t*)data, mTempData, deVideoFrame->width, deVideoFrame->height);
	//rescale video frame
	deVideoFrame->data[0] = (uint8_t*)data;
	AVFrame* swsFrame = CreateVideoFrame(DEFAULTGIFINPUTPIXFMT, enVideoCont->GetAVCodecContext()->width, enVideoCont->GetAVCodecContext()->height);
	if (swsCont->RescaleVideoFrame(deVideoFrame, swsFrame) == false)
		goto end;
	//filter image
	swsFrame->pts = pts++;
	if (filterCont->AddFrame(swsFrame) == false)
		goto end;
	sinkFrame = filterCont->GetFrame();
	if (sinkFrame == nullptr)
		goto end;
	//encode frame
	result = enVideoCont->EncodeFrame(*outfmtCont, videoStream, sinkFrame);
	av_frame_free(&swsFrame);
	return result;
end:
	av_frame_free(&swsFrame);
	return false;
}

void RecordGif::FlipImage(uint8_t* srcData, uint8_t* tempData,int width,int height)
{
	int channel = 4;
	int mSize = width * height * sizeof(char) * channel;

	memcpy(tempData, srcData, mSize);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < channel; k++)
			{
				srcData[(i * width + j) * channel + k] = tempData[((height - 1 - i) * width + j) * channel + k];
			}
		}
	}
}

bool RecordGif::FlushEnVideoCodecBuffer()
{
	AVFrame* sinkFrame = nullptr;
	if (filterCont->FlushBuffer() == false)
		return false;
	do
	{
		sinkFrame = filterCont->GetFrame();
		enVideoCont->EncodeFrame(*outfmtCont, videoStream, sinkFrame);
	} while (sinkFrame != nullptr);
	enVideoCont->FlushBuffer(*outfmtCont, videoStream);
	av_log_info("record gif frame count:%d\n", inFrameCount);
	return true;
}

bool RecordGif::WriteGIFTailer()
{
	return outfmtCont->WriteTofileClosure();
}

RecordGif::~RecordGif()
{
	free(mTempData);
	delete swsCont;
	swsCont = nullptr;
	delete enVideoCont;
	enVideoCont = nullptr;
	delete outfmtCont;
	outfmtCont = nullptr;
	delete filterCont;
	filterCont = nullptr;
}