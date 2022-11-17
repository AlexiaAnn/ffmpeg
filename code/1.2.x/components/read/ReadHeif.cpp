#include "ReadHeif.h"

ReadHeif::ReadHeif(const char* srcPath):path(srcPath),ret(0)
{
	context = heif_context_alloc();
	heif_error error = heif_context_read_from_file(context,path,nullptr);
	if (error.code > 0) ret = -1;
}

int ReadHeif::GetWidth() const
{
	heif_image_handle* handle;
	heif_context_get_primary_image_handle(context, &handle);
	return heif_image_handle_get_width(handle);
}

int ReadHeif::GetHeight() const
{
	heif_image_handle* handle;
	heif_context_get_primary_image_handle(context, &handle);
	return heif_image_handle_get_height(handle);
}

void ReadHeif::ConverHeifToRgb(void* data)
{
	if (GetResult() < 0) return;
	// get a handle to the primary image
	heif_image_handle* handle;
	heif_context_get_primary_image_handle(context, &handle);

	// decode the image and convert colorspace to RGB, saved as 24bit interleaved
	heif_image* img;
	heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);

	int stride;
	const uint8_t* imageData = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
	//FlipImage(const_cast<uint8_t*>(imageData),GetWidth(),GetHeight(),3);
	//copy data
	memcpy(data,imageData,GetHeight()*stride);
}

ReadHeif::~ReadHeif()
{
	if (context == nullptr)return;
	heif_context_free(context);
	context = nullptr;
}

int ReadHeif::GetResult() const
{
	return ret;
}
