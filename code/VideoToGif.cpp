#include "VideoToGif.h"

VideoToGif::VideoToGif(const char* srcFilePath, const char* dstFilePath):pts(0),ret(0)
{
	infmtCont = new InFormatContext(srcFilePath);
	if (infmtCont->GetResult()<0) goto end;
	videoStream = infmtCont->GetVideoStreamByWhile();
	if (videoStream == nullptr) goto end;
	deCodeCont = new DeCodecContext(videoStream);
	if (deCodeCont->GetResult()) goto end;
	enCodeCont = new EnCodecVideoContext(AV_CODEC_ID_GIF,
										deCodeCont->GetCodecContext()->width,
										deCodeCont->GetCodecContext()->height,
										25,
										0.2);
	if (enCodeCont->GetResult()<0) goto end;
	swsCont = new AVSwsContext(deCodeCont->GetCodecContext(),DEFAULTGIFINPUTPIXFMT, deCodeCont->GetCodecContext()->width, deCodeCont->GetCodecContext()->height);
	if (swsCont->GetResult()<0) goto end;
	/*swsFrame = CreateVideoFrame(DEFAULTGIFINPUTPIXFMT, deCodeCont->GetCodecContext()->width, deCodeCont->GetCodecContext()->height);
	if (swsFrame == nullptr) goto end;*/
	filterCont =new FilterContext(enCodeCont->GetAVCodecContext());
	if (filterCont->GetResult()<0) goto end;
	outfmtCont = new OutFormatContext(dstFilePath, { {enCodeCont->GetAVCodecContext(),videoStream} });
	if (outfmtCont->GetResult()<0) goto end;
	return;
	
end:
	ret = -1;
	return;
}

int VideoToGif::GetResult() const
{
	return ret;
}

bool VideoToGif::DoConvert()
{
	outfmtCont->WriteTofilePreparition();
	AVFrame* frame = nullptr;
	AVFrame* sinkFrame = nullptr;
	//ËÍÈëÂË²¨Æ÷
	while ((frame= deCodeCont->GetNextFrame(*infmtCont))!= nullptr) {
		AVFrame* swsFrame = CreateVideoFrame(DEFAULTGIFINPUTPIXFMT, deCodeCont->GetCodecContext()->width, deCodeCont->GetCodecContext()->height);
		if(swsCont->RescaleVideoFrame(frame, swsFrame)==false)continue;
		swsFrame->pts = pts++;
		if (filterCont->AddFrame(swsFrame) == false)continue;
		sinkFrame = filterCont->GetFrame();
		if (sinkFrame==nullptr) continue;
		enCodeCont->EncodeVideoFrame(outfmtCont->GetFormatContext(),videoStream,sinkFrame);
	}
	//³åË¢ÂË²¨Æ÷»º³åÆ÷
	filterCont->FlushBuffer();
	do {
		sinkFrame = filterCont->GetFrame();
		enCodeCont->EncodeVideoFrame(outfmtCont->GetFormatContext(), videoStream, sinkFrame);
	} while (sinkFrame != nullptr);
	outfmtCont->WriteTofileClosure();
	return true;
}

VideoToGif::~VideoToGif()
{
	delete infmtCont;
	infmtCont = nullptr;
	delete deCodeCont;
	deCodeCont = nullptr;
	delete swsCont;
	swsCont = nullptr;
	delete enCodeCont;
	enCodeCont = nullptr;
	delete filterCont;
	filterCont = nullptr;
	delete outfmtCont;
	outfmtCont = nullptr;
}
