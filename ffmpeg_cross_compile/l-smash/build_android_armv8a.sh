NDK=/home/pc/android-ndk-r21e
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/windows-x86_64
API=21
 
 
function build_one
{
./configure \
    --prefix=$PREFIX \
    --enable-shared \
    --cc=$CC \
    --target-os=android \
	--cross-prefix=$CROSS_PREFIX \
    --sysroot=$NDK/toolchains/llvm/prebuilt/windows-x86_64/sysroot \
 
make clean
make -j 32
make install
}
 
#arm64-v8a
PREFIX=lamash-android_r$API
my_host=aarch64-linux-android
TARGET=aarch64-linux-android
CC=clang
CXX=clang++
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android$API-
build_one
 
#armeabi-v7a
# PREFIX=./android/armeabi-v7a
# my_host=armv7a-linux-android
# export TARGET=armv7a-linux-androideabi
# export CC=$TOOLCHAIN/bin/$TARGET$API-clang
# export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
# CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
# build_one