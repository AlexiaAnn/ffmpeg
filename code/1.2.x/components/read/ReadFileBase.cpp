#include "ReadFileBase.h"

ReadFileBase::ReadFileBase(const char *srcFilePath, AVMediaType mediaType) : ret(0)
{
	inFmtCont = new InFormatContext(srcFilePath);
	if (inFmtCont->GetResult() < 0)
		goto end;
	switch (mediaType)
	{
	case AVMEDIA_TYPE_VIDEO:
		deCodeCont = new DeCodecContext(inFmtCont->GetVideoStream());
		break;
	case AVMEDIA_TYPE_AUDIO:
		deCodeCont = new DeCodecContext(inFmtCont->GetAudioStream());
		break;
	default:
		deCodeCont = nullptr;
		goto end;
		break;
	}
	if (deCodeCont->GetResult() < 0)
		goto end;
	ret = 0;
	return;
end:
	ret = -1;
	av_log_error("ReadFileBase initialize failed\n");
	return;
}

int ReadFileBase::GetResult() const
{
	return ret;
}

ReadFileBase::~ReadFileBase()
{
	av_log_info("ReadFileBase deConstructor\n");
	delete inFmtCont;
	inFmtCont = nullptr;
	delete deCodeCont;
	deCodeCont = nullptr;
}
