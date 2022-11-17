#include "VideoEncoderThread.h"

VideoEncoderThread::VideoEncoderThread(AVCodecID encodeId, int width, int height, int fps,
	float bitRatePercent, int crfMin, int crfMax, int presetLevel) :
	EnCodecVideoContext(encodeId, width, height, fps, bitRatePercent, crfMin, crfMax, presetLevel),
	mIsEnd(false)
{
}

VideoEncoderThread::VideoEncoderThread(EncodeName encodeName, int width, int height, int fps, 
			float bitRatePercent, int crfMin, int crfMax, int presetLevel):
	EnCodecVideoContext(encodeName, width, height, fps, bitRatePercent, crfMin, crfMax, presetLevel),
	mIsEnd(false)
{
}

void VideoEncoderThread::ThreadStart(OutFormatContext* outFmtCont, AVStream* outStream)
{
	mOFormat = outFmtCont;
	mVideoStream = outStream;
	

	mThread = std::thread([this]() {
		av_log_info("encoder thread start");
		while (true) {
			AVFrame* frameInQueue = mQueue.Pop();
			if (frameInQueue == nullptr) {
				EnCodecVideoContext::EncodeFrame(*mOFormat, mVideoStream, nullptr);
				av_log_info("encoder thread end");
				return;
			}
			memcpy(frame->data[0], frameInQueue->data[0], frame->linesize[0] * frame->height);
			memcpy(frame->data[1], frameInQueue->data[1], frame->linesize[1] * frame->height / 2);
			memcpy(frame->data[2], frameInQueue->data[2], frame->linesize[2] * frame->height / 2);
			EnCodecVideoContext::EncodeFrame(*mOFormat, mVideoStream);
			av_frame_free(&frameInQueue);
		}
		
		});
}


bool VideoEncoderThread::EncodeFrame(int id, AVFrame* frame)
{
	mQueue.Push({id,frame});
	return true;
}


bool VideoEncoderThread::FlushBuffer(OutFormatContext& outFmtCont, AVStream* outStream)
{
	mQueue.Push({1000000,nullptr});
	mThread.join();
	return true;
}

VideoEncoderThread::~VideoEncoderThread()
{
}
