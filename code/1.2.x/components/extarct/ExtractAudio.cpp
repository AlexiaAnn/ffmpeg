#include "ExtractAudio.h"

ExtractAudio::ExtractAudio():inFmtContPointer(nullptr),deCodeContPointer(nullptr),
							 enCodeContPointer(nullptr),outFmtContPointer(nullptr),
							 deAudioStream(nullptr),ret(-1)
{

}

ExtractAudio::ExtractAudio(const char* srcFilePath, const char* dstFilePath):ret(0)
{
	inFmtContPointer = new InFormatContext(srcFilePath);
	if (inFmtContPointer->GetResult() < 0) goto end;
	deAudioStream = inFmtContPointer->GetAudioStream();
	if (deAudioStream == nullptr) {
		av_log_error("no audio stream in file\n");
		goto end;
	}
	deCodeContPointer = new DeCodecContext(deAudioStream);
	if (deCodeContPointer->GetResult() < 0)goto end;

	enCodeContPointer = new EnCodecAudioContext(DEFAULTAUDIOCODECID);
	if (enCodeContPointer->GetResult() < 0) goto end;

	swrContPointer = new AVSwrContext(deCodeContPointer->GetCodecContext(),enCodeContPointer->GetAVCodecContext());
	if (swrContPointer->GetResult() < 0) goto end;

	outFmtContPointer = new OutFormatContext(dstFilePath, { {enCodeContPointer->GetAVCodecContext(),audioStream} });
	if (outFmtContPointer->GetResult() < 0) goto end;
	return;
end:
	ret = -1;
	return;
}

void ExtractAudio::DoExtract()
{
	if (ret < 0) {
		av_log_error("Problem with initialization of ExtractAudio context,can`t extract audio\n");
		return;
	}
	AVFrame* deFrame;
	outFmtContPointer->WriteTofilePreparition();
	while ((deFrame = deCodeContPointer->GetNextFrame(*inFmtContPointer))!=nullptr) {
		//resample
		if (swrContPointer->ResampleAudioFrame(deFrame, *enCodeContPointer) == false) continue;
		//encode
		enCodeContPointer->EncodeFrame(*outFmtContPointer, audioStream);
	}
	enCodeContPointer->FlushBuffer(*outFmtContPointer, audioStream);
	outFmtContPointer->WriteTofileClosure();
}

int ExtractAudio::GetResult() const
{
	return ret;
}

ExtractAudio::~ExtractAudio()
{
	delete inFmtContPointer;
	inFmtContPointer = nullptr;
	delete outFmtContPointer;
	outFmtContPointer = nullptr;
	delete deCodeContPointer;
	deCodeContPointer = nullptr;
	delete enCodeContPointer;
	enCodeContPointer = nullptr;
}
