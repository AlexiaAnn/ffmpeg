#!/bin/bash
set -x
##########################根据自己的电脑环境修改######################Start
API=21
#armv8-a，armv7-a，i686，x86-64
ARCH=arm64
#armv8-a，armv7-a，i686，x86-64
CPU=armv8-a
#aarch64，armv7a, i686, x86_64
TOOL_CPU_NAME=aarch64
#so file output dir
HOME=/home/alexia/ffmpeg
OUTPUT=./android/$CPU
NDK=/home/alexia/ffmpeg/android-ndk-r21e
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android21-
TOOL_PREFIX="$TOOLCHAIN/bin/$TOOL_CPU_NAME-linux-android"
CC="$TOOL_PREFIX$API-clang"
CXX="$TOOL_PREFIX$API-clang++"
STRIP="$TOOL_PREFIX-strip"
#if build armv7，use this
#STRIP="$TOOLCHAIN/bin/arm-linux-androideabi-strip"
#编译好的H264文件位置
COMPILE_H264_PATH=$HOME/x264-master/android/arm64-v8a
COMPILE_MP3_PATH=$HOME/libmp3lame
######################################################################End


OPTIMIZE_CFLAGS="-march=$CPU"
function build
{
  ./configure \
  --prefix=$OUTPUT \
  --target-os=android \
  --arch=$ARCH  \
  --cpu=$CPU \
  --disable-asm \
  --disable-stripping \
  --disable-programs \
  --disable-static \
  --disable-doc \
  --disable-ffplay \
  --disable-ffprobe \
  --disable-symver \
  --disable-ffmpeg \
  --enable-shared \
  --enable-cross-compile \
  --enable-libmp3lame \
  --enable-encoder=libmp3lame \
  --enable-libx264 \
  --enable-encoder=libx264 \
  --enable-gpl \
  --cc=$CC \
  --cxx=$CXX \
  --strip=$STRIP \
  --sysroot=$SYSROOT \
  --extra-libs="-pthread -lm" \
  --extra-cflags="-Os -fpic -I/home/alexia/ffmpeg/x264-master/android/arm64-v8a/include -I$COMPILE_MP3_PATH/include $OPTIMIZE_CFLAGS " \
  --extra-ldflags="-L/home/alexia/ffmpeg/x264-master/android/arm64-v8a/lib -L$COMPILE_MP3_PATH/libs/armv8-a"

  make clean all
  # 这里是定义用几个CPU编译
  make -j8
  make install
}
build