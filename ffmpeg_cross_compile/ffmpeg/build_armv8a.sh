#!/bin/bash
set -x
##########################根据自己的电脑环境修改######################Start
API=21
#so file output dir
HOME=/home/pc
OUTPUT=libffmpeg_android_r21e_mediacodec_libmp3lame_libx264
NDK=$HOME/android-ndk-r21e
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/windows-x86_64
SYSROOT=$TOOLCHAIN/sysroot
#if build armv7，use this
#STRIP="$TOOLCHAIN/bin/arm-linux-androideabi-strip"

######################################################################End


OPTIMIZE_CFLAGS="-march=$CPU"
function build
{
  ./configure \
  --prefix=$OUTPUT \
  --target-os=android \
  --arch=$ARCH  \
  --cpu=$CPU \
  --enable-neon \
  --disable-optimizations \
  --disable-stripping \
  --disable-programs \
  --disable-static \
  --disable-doc \
  --enable-shared \
  --enable-debug \
  --enable-cross-compile \
  --enable-libmp3lame \
  --enable-encoder=libmp3lame \
  --enable-libx264 \
  --enable-encoder=libx264 \
  --enable-gpl \
  --enable-mediacodec \
  --enable-hlmediacodec \
  --enable-postproc \
  --enable-pic \
  --enable-zlib \
  --disable-bzlib \
  --disable-symver \
  --disable-iconv \
  --enable-hwaccels  \
  --enable-hwaccel=h264_mediacodec \
  --enable-encoder=h264_hlmediacodec \
  --enable-libx264 \
  --enable-encoder=libx264 \
  --enable-jni \
  --cc=$CC \
  --cxx=$CXX \
  --strip=$STRIP \
  --sysroot=$SYSROOT \
  --extra-libs="-pthread -lm -lmediandk" \
  --extra-cflags="-Os -fpic -I$COMPILE_H264_PATH/include -I$COMPILE_MP3_PATH/include $OPTIMIZE_CFLAGS $COMPILE_MEDIA_SO_INCLUDE" \
  --extra-ldflags="-L$COMPILE_H264_PATH/lib -L$COMPILE_MP3_PATH/arm64-v8a -L$COMPILE_MEDIA_SO_LIB"

  # make clean all
  # ##这里是定义用几个CPU编译
  # make -j32
  # make install
}

# ****************armv8a*********************
#armv8-a，armv7-a，i686，x86-64
ARCH=arm64
#armv8-a，armv7-a，i686，x86-64
CPU=armv8-a
#aarch64，armv7a, i686, x86_64
TOOL_CPU_NAME=aarch64
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android21-
TOOL_PREFIX="$TOOLCHAIN/bin/$TOOL_CPU_NAME-linux-android"
CC="$TOOL_PREFIX$API-clang"
CXX="$TOOL_PREFIX$API-clang++"
STRIP="$TOOL_PREFIX-strip"
#编译好的H264文件位置
COMPILE_H264_PATH=$HOME/x264/x264_android_r21
export PKG_CONFIG_PATH=$COMPILE_H264_PATH/lib/pkgconfig
COMPILE_MP3_PATH=$HOME/libmp3lame/libs
COMPILE_MEDIA_SO_LIB=$NDK/platforms/android-21/arch-arm64/usr/lib
COMPILE_MEDIA_SO_INCLUDE=$TOOLCHAIN/sysroot/usr/include/media
build
# ****************armv7a*********************
#armv8-a，armv7-a，i686，x86-64
# ARCH=arm
# #armv8-a，armv7-a，i686，x86-64
# CPU=armv7-a
# #aarch64，armv7a, i686, x86_64
# TOOL_CPU_NAME=armv7a
# CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-android21-
# TOOL_PREFIX="$TOOLCHAIN/bin/$TOOL_CPU_NAME-linux-android"
# CC="$TOOL_PREFIX$API-clang"
# CXX="$TOOL_PREFIX$API-clang++"
# STRIP="$TOOL_PREFIX-strip"
# #编译好的H264文件位置
# COMPILE_MP3_PATH=$HOME/libmp3lame/libs
# COMPILE_MEDIA_SO_LIB=$NDK/platforms/android-21/arch-arm/usr/lib
# COMPILE_MEDIA_SO_INCLUDE=$TOOLCHAIN/sysroot/usr/include/media
# build
#arm64，arm,x86,x86_64
# ARCH=arm
# #armv8-a，armv7-a，x86，x86-64
# CPU=armv8-a
# #aarch64，armv7a, i686, x86_64
# TOOL_CPU_NAME=armv7a
# CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-android21-
# TOOL_PREFIX="$TOOLCHAIN/bin/$TOOL_CPU_NAME-linux-androideabi"
# CC="$TOOL_PREFIX$API-clang"
# CXX="$TOOL_PREFIX$API-clang++"
# STRIP="$TOOL_PREFIX-strip"
# COMPILE_H264_PATH=$HOME/libx264_android_r21e
# export PKG_CONFIG_PATH=$COMPILE_H264_PATH/lib/pkgconfig
# COMPILE_MP3_PATH=$HOME/libmp3lame/libs/armeabi-v7a
# build
