#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif // ANDROID
#include <vector>
class OutFormatContext
{
private:
	AVFormatContext *fmtCont = nullptr;
	int ret = 0;

private:
	AVStream *AddNewStreamToFormat(AVFormatContext *fmtCont, AVCodecContext *codeCont);

public:
	OutFormatContext();
	OutFormatContext(const char *dstFilePath, std::vector<std::pair<AVCodecContext *, AVStream *&>> codeContVector);
	bool WriteTofilePreparition();
	bool WriteTofileClosure();
	AVFormatContext *GetFormatContext() const;
	int GetResult() const;
	~OutFormatContext();
};
