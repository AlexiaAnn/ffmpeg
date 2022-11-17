#pragma once
#ifdef WINDOWS
#include "../utils/util.h"
#endif // WINDOWS
#ifdef ANDROID
#include "utils/util.h"
#endif
class ReadHeif
{
private:
	const char* path;
	heif_context* context = nullptr;
	int ret;
public:
	ReadHeif() = delete;
	ReadHeif(const char* srcPath);
	int GetWidth() const;
	int GetHeight() const;
	void ConverHeifToRgb(void* data);
	int GetResult() const;
	~ReadHeif();
};

