#include "ExtractVideo.h"

ExtractVideo::ExtractVideo():ExtractBase()
{
}

ExtractVideo::ExtractVideo(const char* srcFilePath, const char* dstFilePath):ExtractBase(srcFilePath)
{
	if (ret < 0) return;

	deStream = inFmtContPointer->GetVideoStream();
	if (deStream == nullptr) {
		av_log_error("no video stream in file\n");
		goto end;
	}
	deCodeContPointer = new DeCodecContext(deStream);
	if (deCodeContPointer->GetResult() < 0)goto end;

	enCodeContPointer = new EnCodecVideoContext(DEFAULTCODECID,
												deStream->codecpar->width,
												deStream->codecpar->height,
												deStream->avg_frame_rate.num/deStream->avg_frame_rate.den,
												DEFAULTBITRATEPERCENT,DEFAULTCRFMIN,DEFAULTCRFMAX,0);
	if (enCodeContPointer->GetResult() < 0) goto end;

	swsContPointer = new AVSwsContext(deCodeContPointer->GetCodecContext(),enCodeContPointer->GetAVCodecContext());
	if (swsContPointer->GetResult() < 0) goto end;

	outFmtContPointer = new OutFormatContext(dstFilePath, { {enCodeContPointer->GetAVCodecContext(),enStream} });
	if (outFmtContPointer->GetResult() < 0) goto end;
	return;
end:
	ret = -1;
	av_log_error("ExtractVideo context initialize failed\n");
	return;

}

void ExtractVideo::DoExtract()
{
	if (ret < 0) {
		av_log_error("Problem with initialization of ExtractAudio context,can`t extract audio\n");
		return;
	}
	AVPacket* packet;
	AVFrame* deFrame;
	outFmtContPointer->WriteTofilePreparition();
	while ((deFrame = deCodeContPointer->GetNextFrame(*inFmtContPointer)) != nullptr) {
		if (swsContPointer->IsNeedRescale() == false) {
			enCodeContPointer->EncodeFrame(*outFmtContPointer, enStream,deFrame);
		}
		else {
			//rescale
			if (swsContPointer->RescaleVideoFrame(deFrame, *enCodeContPointer) == false) continue;
			//encode
			enCodeContPointer->EncodeFrame(*outFmtContPointer, enStream);
		}
		
	}
	enCodeContPointer->FlushBuffer(*outFmtContPointer, enStream);
	outFmtContPointer->WriteTofileClosure();
}

ExtractVideo::~ExtractVideo()
{
	delete outFmtContPointer;
	outFmtContPointer = nullptr;
	delete deCodeContPointer;
	deCodeContPointer = nullptr;
	delete enCodeContPointer;
	enCodeContPointer = nullptr;
}
