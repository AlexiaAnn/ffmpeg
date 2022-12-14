#include "AVSwsContext.h"

AVSwsContext::AVSwsContext(AVPixelFormat dePixFormat, int deWidth, int deHeight, AVPixelFormat enPixFormat, int enWidth, int enHeight):swsCont(nullptr),ret(0)
{
	if (dePixFormat == enPixFormat && deWidth == enWidth && deHeight == enHeight) {
		swsCont = nullptr;
		av_log_info("AVSwsContext is unnecessary\n");
	}
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

bool AVSwsContext::IsNeedRescale() const
{
	return swsCont != nullptr;
}

bool AVSwsContext::RescaleVideoFrame(AVFrame* deVideoFrame, AVFrame* enVideoFrame)
{
	if (swsCont == nullptr) {
		av_log_warning("swscontext is nullptr,cant rescale video frame\n");
		return false;
	}
	start = clock();
	ret = sws_scale(swsCont, deVideoFrame->data, deVideoFrame->linesize, 0,
				    deVideoFrame->height, enVideoFrame->data, enVideoFrame->linesize);
	end = clock();
	swsTime += float(end - start) / CLOCKS_PER_SEC;
	frameCount += 1;
	if (ret <= 0) {
		char errorStr[AV_ERROR_MAX_STRING_SIZE];
		av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE,ret);
		av_log_error("sws scale the video frame failed,ret code:%s", errorStr);
		return false;
	}
	return true;
}

bool AVSwsContext::RescaleVideoFrame(AVFrame* deVideoFrame, EnCodecVideoContext& codeCont)
{
	if (swsCont == nullptr) {
		av_log_warning("swscontext is nullptr,cant rescale video frame\n");
		return false;
	}
	AVFrame* enVideoFrame = codeCont.GetEncodecFrame();
	if (enVideoFrame == nullptr) {
		av_log_warning("enframe is nullptr,cant to rescale\n");
		return false;
	}
	start = clock();
	ret = sws_scale(swsCont, deVideoFrame->data, deVideoFrame->linesize, 0,
		deVideoFrame->height, enVideoFrame->data, enVideoFrame->linesize);
	end = clock();
	swsTime += float(end - start) / CLOCKS_PER_SEC;
	frameCount += 1;
	if (ret <= 0) {
		char errorStr[AV_ERROR_MAX_STRING_SIZE];
		av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, ret);
		av_log_error("sws scale the video frame failed,ret code:%s", errorStr);
		return false;
	}
	return true;
}

void AVSwsContext::GetTimeInfo() const
{
	av_log_info("test time=>[sws time:%fs,avg:%fs]",swsTime,swsTime/frameCount);
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
