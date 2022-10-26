#pragma once
#include "IncludeFFmpeg.h"
class TestContext {
private:
	AVFormatContext* context;
	AVDictionary* format_opts;
	AVDictionaryEntry* t;
	AVPacket* packet;
	int scan_all_pmts_set;
	int ret;
public:
	TestContext(const char* srcFilePath);
	void main();
};