#include "EnCodecContext.h"

EnCodecContext::EnCodecContext():codecCont(nullptr),frame(nullptr),packet(nullptr),pts(0),ret(0)
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
	/*av_log_info("%s start",__FUNCTION__);
	if (codecCont == nullptr) return;
	avcodec_free_context(&codecCont);
	codecCont = nullptr;
	av_log_info("%s end", __FUNCTION__);*/
}
