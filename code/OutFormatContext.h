#pragma once
#include "util.h"
#include <vector>
class OutFormatContext
{
private:
	AVFormatContext* fmtCont;
	int ret;
private:
	AVStream* AddNewStreamToFormat(AVFormatContext* fmtCont, AVCodecContext* codeCont);
public:
	OutFormatContext();
	OutFormatContext(const char* dstFilePath,std::vector<std::pair<AVCodecContext*, AVStream*&>> codeContVector);
	bool WriteTofilePreparition();
	bool WriteTofileClosure();
	AVFormatContext* GetFormatContext() const;
	int GetResult() const;
	~OutFormatContext();
};

