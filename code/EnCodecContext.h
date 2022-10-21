#pragma once
#include "util.h"

class EnCodecContext
{
protected:
	AVCodecContext* codecCont;
	int ret;
public:
	EnCodecContext();
	int GetResult() const;
	AVCodecContext* GetAVCodecContext() const;
	virtual ~EnCodecContext();
};

