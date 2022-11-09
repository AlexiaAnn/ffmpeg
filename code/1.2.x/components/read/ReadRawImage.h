#pragma once
#include ""
class ReadRawImage
{
private:
	int size;
	const char* mSrcFilePath;
	unsigned char* data;
	FILE* srcFilePtr;
	int ret;
public:
	ReadRawImage(AVPixelFormat format,int width,int height,const char* srcFilePath);
	unsigned char* GetRawData();
	~ReadRawImage();
};

