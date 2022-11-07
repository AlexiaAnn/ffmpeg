HOME=/home/pc
NDK=/home/pc/android-ndk-r21e
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/windows-x86_64
API=21
 
 
function build_one
{
./configure \
    --prefix=$PREFIX \
    --enable-static \
    --enable-pic \
    --enable-debug \
    --disable-ffms \
    --disable-swscale \
    --disable-lavf \
    --host=aarch64-linux-android \
	--cross-prefix=$CROSS_PREFIX \
    --sysroot=$NDK/toolchains/llvm/prebuilt/windows-x86_64/sysroot \
    --extra-cflags="-Os -fpic -pthread -I$LSMASHINCLUDE" \
    --extra-ldflags="-L$LSMASHLIB -pthread"
 
make clean
make -j 32
make install
}
#arm64-v8a
PREFIX=x264_android_r$API
my_host=aarch64-linux-android
TARGET=aarch64-linux-android
CC=clang
CXX=clang++
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android$API-
LSMASHPATH=$HOME/l-smash/lsmash-android_r21
LSMASHLIB=$LSMASHPATH/lib
LSMASHINCLUDE=$LSMASHPATH/include
build_one
 
#armeabi-v7a
# PREFIX=./android/armeabi-v7a
# my_host=armv7a-linux-android
# export TARGET=armv7a-linux-androideabi
# export CC=$TOOLCHAIN/bin/$TARGET$API-clang
# export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
# CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
# build_one