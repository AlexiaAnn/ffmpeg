#include "ReadRawImage.h"

ReadRawImage::ReadRawImage(AVPixelFormat format, int width, int height, const char* srcFilePath):mSrcFilePath(srcFilePath)
{
	size = width * height * 4;
	data = new unsigned char[size];
	srcFilePtr = fopen(srcFilePath, "rb");

}

unsigned char* ReadRawImage::GetRawData()
{
	int len = fread(data,1,size,srcFilePtr);
	if (len <= 0) return nullptr;
	return data;
}

ReadRawImage::~ReadRawImage()
{
	delete data;
	data = nullptr;
}
