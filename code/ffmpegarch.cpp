//#include "AVFileUnityTest.h"
//#include "VideoFileContext.h"
//#include "AVPlayer.h"
//#include "NewPlayer.h"
#include "SeekComponent.h"
//#include "AudioWave.h"
//#include "TestContext.h"
#include "SeekVideo.h"
//#include "ExtractAudioFromMp4.h"
#include "ExtractAudio.h"
#include "ExtractVideo.h"
#include "RecordMp4.h"
#include "ReadRawImage.h"
int main()
{
	//av_log_set_callback(WindowsCallbackFunc);
	const char *srcFilePath = "f:/rgbs/laji.rgb";
	const char *dstFilePath = "f:/audiotest/laji.mp4";
	const int width = 288;
	const int height = 640;
	 /*AVFileUnityTest context;
	 context.Test(srcFilePath, dstFilePath);*/
	/*VideoFileContext context(30,864,1920);
	context.ExtractVideoToFile(srcFilePath,dstFilePath);*/
	/*AudioFileContext context(DEFAULTAUDIOCODECID);
	context.ExtractAudioToFile(srcFilePath,dstFilePath);*/
	/*ExtractAudioFromMp4 context(AV_CODEC_ID_MP3);
	context.ExtractAudioToFile(srcFilePath,dstFilePath);*/
	/*NewPlayer player(srcFilePath);
	player.GetInformation();*/
	/*FileContextBase context(srcFilePath);
	context.GetInformation();*/
	/*SeekVideo component(srcFilePath);
	av_log_info("duration secondes:%f", component.GetDuration());
	component.GetFrameDataByPercent(0, nullptr, 0);
	component.GetFrameDataByPercent(0.004, nullptr,0);
	component.GetFrameDataByPercent(0.4,nullptr,0);
	component.GetFrameDataByPercent(0.6, nullptr,0);
	component.GetFrameDataByPercent(0.9, nullptr,0);
	component.GetFrameDataByPercent(0.1, nullptr,0);*/
	/*AudioWave wave(srcFilePath);
	if (wave.GetRet() < 0) {
		av_log_error("initialize audiowave failed\n");
		return 0;
	}
	wave.GetAudioInformation();*/

	/*TestContext context(srcFilePath);
	context.main();*/

	/*ExtractAudio context(srcFilePath,dstFilePath);
	context.DoExtract();*/

	/*ExtractVideo context(srcFilePath,dstFilePath);
	context.DoExtract();*/

	//rgba to mp4
	unsigned char* data = nullptr;
	ReadRawImage read(AV_PIX_FMT_RGBA,width,height,srcFilePath);
	RecordMp4 record(dstFilePath,48000,AV_SAMPLE_FMT_FLT,MONOLAYOUT,AV_PIX_FMT_RGBA,25,0.2,width,height);
	record.WriteAVPreparition();
	while ((data = read.GetRawData()) != nullptr) {
		record.WriteVideoToFile(data,0);
	}
	record.FlushEnVideoCodecBuffer();
	record.WriteAVTailer();
}