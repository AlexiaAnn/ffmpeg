#include "AVFileUnityTest.h"
#include "VideoFileContext.h"
#include "AVPlayer.h"
#include "NewPlayer.h"
#include "SeekComponent.h"
#include "AudioWave.h"
#include "TestContext.h"
#include "SeekVideo.h"
int main()
{
	const char *srcFilePath = "f:/mouse.mp4";
	const char *dstFilePath = "f:/mouse.mp3";
	 /*AVFileUnityTest context;
	 context.Test(srcFilePath, dstFilePath);*/
	/*VideoFileContext context(30,864,1920);
	context.ExtractVideoToFile(srcFilePath,dstFilePath);*/
	/*AudioFileContext context(DEFAULTAUDIOCODECID);
	context.ExtractAudioToFile(srcFilePath,dstFilePath);*/
	/*NewPlayer player(srcFilePath);
	player.GetInformation();*/
	/*FileContextBase context(srcFilePath);
	context.GetInformation();*/
	SeekVideo component(srcFilePath);
	av_log_info("duration secondes:%f", component.GetDuration());
	component.GetFrameDataByPercent(0.004, nullptr,0);
	component.GetFrameDataByPercent(0.4,nullptr,0);
	component.GetFrameDataByPercent(0.6, nullptr,0);
	component.GetFrameDataByPercent(0.9, nullptr,0);
	component.GetFrameDataByPercent(0.1, nullptr,0);
	/*AudioWave wave(srcFilePath);
	if (wave.GetRet() < 0) {
		av_log_error("initialize audiowave failed\n");
		return 0;
	}
	wave.GetAudioInformation();*/

	/*TestContext context(srcFilePath);
	context.main();*/
}