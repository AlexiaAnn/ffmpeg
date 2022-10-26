#include "AVFileContext.h"
#include "AVContext.h"
class AVFileUnityTest : public AVFileContext
{
private:
    int isOtherStream;
    AVContext* avContext;
    void DealAudioPacket() override;
    void DealVideoPacket() override;

public:
    AVFileUnityTest();
    void Test(const char *srcFilePath, const char *dstFilePath);
    ~AVFileUnityTest();
};