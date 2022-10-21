#include "EnCodecContext.h"

EnCodecContext::EnCodecContext():codecCont(nullptr),ret(0)
{
}


int EnCodecContext::GetResult() const
{
	return 0;
}

AVCodecContext* EnCodecContext::GetAVCodecContext() const
{
	return codecCont;
}

EnCodecContext::~EnCodecContext()
{
}
