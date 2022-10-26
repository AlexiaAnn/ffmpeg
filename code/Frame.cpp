#include "Frame.h"

Frame::~Frame()
{
	av_frame_unref(frame);
	av_frame_free(&frame);
}
