#include "ExtractBase.h"

ExtractBase::ExtractBase():ret(-1)
{

}

ExtractBase::ExtractBase(const char* srcFilePath):ret(0)
{
	inFmtContPointer = new InFormatContext(srcFilePath);
	if (inFmtContPointer->GetResult() < 0) {
		ret = -1;
		av_log_error("InFormatContext initialize failed\n");
		return;
	}
}


int ExtractBase::GetResult() const
{
	return ret;
}

ExtractBase::~ExtractBase()
{
	delete inFmtContPointer;
	inFmtContPointer = nullptr;
	delete outFmtContPointer;
	outFmtContPointer = nullptr;
}
