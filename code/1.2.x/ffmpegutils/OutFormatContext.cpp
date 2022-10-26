#include "OutFormatContext.h"

AVStream* OutFormatContext::AddNewStreamToFormat(AVFormatContext* fmtCont, AVCodecContext* codeCont)
{
    AVStream* stream = avformat_new_stream(fmtCont, nullptr);
    if (stream == nullptr)
    {
        av_log_error("create a stream to formatcontext failed\n");
        return nullptr;
    }
    int ret = avcodec_parameters_from_context(stream->codecpar, codeCont);
    if (ret < 0)
    {
        av_log_error("copy codec parameters from context failed\n");
        return nullptr;
    }
    stream->id = fmtCont->nb_streams - 1;
    stream->codecpar->codec_tag = 0;
    stream->time_base = codeCont->time_base; // error prone
    return stream;
}

OutFormatContext::OutFormatContext():fmtCont(nullptr),ret(0)
{
}

OutFormatContext::OutFormatContext(const char* dstFilePath, std::vector<std::pair<AVCodecContext*, AVStream*&>> codeContVector)
{
	fmtCont = AllocOutFormatContext(dstFilePath);
	if (fmtCont == nullptr) {
		goto end;
	}
    //add stream to format context
    for (std::pair<AVCodecContext*, AVStream*&> item : codeContVector) {
        AVStream* tempStream = AddNewStreamToFormat(fmtCont, item.first);
        if (tempStream==nullptr) {
            av_log_error("one stream failed to add\n");
            item.second = nullptr;
        }
        else {
            item.second = tempStream;
        }
    }
	return;
end:
	ret = -1;
	av_log_error("OutFormatContext initialize failed\n");
	return;
}

bool OutFormatContext::WriteTofilePreparition()
{
    ret = avio_open(&fmtCont->pb, fmtCont->url, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        av_log_error("avio open failed\n");
        return false;
    }
    ret = avformat_write_header(fmtCont, nullptr);
    if (ret < 0)
    {
        av_log_error("write header information failed\n");
        return false;
    }
    return true;
}

bool OutFormatContext::WriteTofileClosure()
{
    ret = av_write_trailer(fmtCont);
    if (ret < 0)
    {
        av_log_error("write trailer information failed\n");
        ret = avio_close(fmtCont->pb);
        if (ret < 0)
        {
            av_log_error("close outfmt context failed\n");
            return false;
        }
        return false;
    }
    ret = avio_close(fmtCont->pb);
    if (ret < 0)
    {
        av_log_error("close outfmt context failed\n");
        return false;
    }
    return true;
}

AVFormatContext* OutFormatContext::GetFormatContext() const
{
    return fmtCont;
}



int OutFormatContext::GetResult() const
{
	return 0;
}

OutFormatContext::~OutFormatContext()
{
    avformat_free_context(fmtCont);
    fmtCont = nullptr;
}
