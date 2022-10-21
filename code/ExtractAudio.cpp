#include "ExtractAudio.h"

ExtractAudio::ExtractAudio():inFmtContPointer(nullptr),deCodeContPointer(nullptr),
							 enCodeContPointer(nullptr),outFmtContPointer(nullptr),
							 deAudioStream(nullptr)
{

}

ExtractAudio::ExtractAudio(const char* srcFilePath, const char* dstFilePath)
{
	inFmtContPointer = new InFormatContext(srcFilePath);
	if (inFmtContPointer->GetResult() < 0) goto end;
	deAudioStream = inFmtContPointer->GetAudioStream();
	deCodeContPointer = new DeCodecContext(deAudioStream);
	if (deCodeContPointer->GetResult() < 0)goto end;

	enCodeContPointer = new EnCodecAudioContext(DEFAULTAUDIOCODECID);
	if (enCodeContPointer->GetResult() < 0) goto end;

	swrContPointer = new AVSwrContext(deCodeContPointer->GetCodecContext(),enCodeContPointer->GetAVCodecContext());
	if (swrContPointer->GetResult() < 0) goto end;

	outFmtContPointer = new OutFormatContext(dstFilePath, { {enCodeContPointer->GetAVCodecContext(),audioStream} });
	if (outFmtContPointer->GetResult() < 0) goto end;
end:
	ret = -1;
	return;
}

void ExtractAudio::DoExtract()
{
	AVPacket* packet;
	AVFrame* deFrame;
	outFmtContPointer->WriteTofilePreparition();
	while ((packet = inFmtContPointer->GetNextPacket()) != nullptr) {
		if (packet->stream_index != deAudioStream->index) continue;
		deCodeContPointer->SendPacket(packet);
		if ((deFrame = deCodeContPointer->GetReceiveFrame()) == nullptr) continue;
		//resample
		if (swrContPointer->ResampleAudioFrame(deFrame, enCodeContPointer->GetEncodecFrame()) == false) continue;
		//encode
		enCodeContPointer->EncodeAudioFrame(outFmtContPointer->GetFormatContext(),audioStream);
	}
	enCodeContPointer->FlushBuffer(outFmtContPointer->GetFormatContext(), audioStream);
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
