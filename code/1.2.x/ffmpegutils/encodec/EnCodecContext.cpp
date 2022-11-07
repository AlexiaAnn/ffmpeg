#include "EnCodecContext.h"

EnCodecContext::EnCodecContext():codecCont(nullptr),frame(nullptr),packet(nullptr),pts(0),ret(-1)
{
}


int EnCodecContext::GetResult() const
{
	return ret;
}

AVCodecContext* EnCodecContext::GetAVCodecContext() const
{
	return codecCont;
}

EnCodecContext::~EnCodecContext()
{
}
