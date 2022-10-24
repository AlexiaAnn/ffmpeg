#include "AVSwsContext.h"

AVSwsContext::AVSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight, AVPixelFormat enPixFormat, int enWidth, int enHeight):swsCont(nullptr),ret(0)
{
	if (dePixFormat == enPixFormat && deWidth == enWidth && deHeight == enHeight) swsCont = nullptr;
	else {
		av_log_info("get swscontext start\n");
		swsCont = sws_getCachedContext(swsCont, deWidth, deHeight, dePixFormat,
									   enWidth, enHeight, enPixFormat,
									   SWS_BICUBIC, nullptr, nullptr, nullptr);
		if (swsCont == nullptr) {
			av_log_error("failed when getting the swscontext,AVSwsContext initialize failed\n");
			ret = -1;
			return;
		}
		av_log_info("get swscontext end\n");
	}
}

AVSwsContext::AVSwsContext(AVCodecContext* deCodecont, AVPixelFormat enPixFormat, int enWidth, int enHeight):
AVSwsContext(deCodecont->pix_fmt,deCodecont->width,deCodecont->height,enPixFormat,enWidth,enHeight)
{
}

AVSwsContext::AVSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight, AVCodecContext* enCodecont) :
	AVSwsContext(dePixFormat, deWidth, deHeight, enCodecont->pix_fmt, enCodecont->width, enCodecont->height)
{
}

AVSwsContext::AVSwsContext(AVCodecContext* deCodecont, AVCodecContext* enCodecont) :
	AVSwsContext(deCodecont->pix_fmt, deCodecont->width, deCodecont->height, enCodecont->pix_fmt, enCodecont->width, enCodecont->height)
{
}

bool AVSwsContext::RescaleVideoFrame(AVFrame* deVideoFrame, AVFrame* enVideoFrame)
{
	if (swsCont == nullptr) {
		av_log_warning("swscontext is nullptr,cant rescale video frame\n");
		return false;
	}
	ret = sws_scale(swsCont, deVideoFrame->data, deVideoFrame->linesize, 0,
				    deVideoFrame->height, enVideoFrame->data, enVideoFrame->linesize);
	if (ret <= 0) {
		char errorStr[AV_ERROR_MAX_STRING_SIZE];
		av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE,ret);
		av_log_error("sws scale the video frame failed,ret code:%s", errorStr);
		return false;
	}
	return true;
}

AVSwsContext::~AVSwsContext()
{
	sws_freeContext(swsCont);
	swsCont = nullptr;
}

int AVSwsContext::GetResult() const
{
	return ret;
}
