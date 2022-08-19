LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := avcodec 
LOCAL_SRC_FILES := libavcodec.so 
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat 
LOCAL_SRC_FILES := libavformat.so 
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample 
LOCAL_SRC_FILES := libswresample.so 
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := libavutil.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := libswscale.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE     :=  ffmpeg_learn
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_SRC_FILES  :=  ffmpeg_learn.cpp MP42MP3Context.cpp util.cpp
LOCAL_SHARED_LIBRARIES := avcodec avformat swresample swscale avutil
LOCAL_LDLIBS     := -llog -landroid
LOCAL_CFLAGS    := -DANDROID_NDK
LOCAL_CPPFLAGS += -fexceptions
include $(BUILD_SHARED_LIBRARY)