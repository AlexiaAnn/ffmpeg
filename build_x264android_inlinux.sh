
export NDK=/home/alexia/ffmpeg/android-ndk-r21e
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
export API=21
 
 
function build_one
{
./configure \
    --prefix=$PREFIX \
	--disable-cli \
    --enable-static \
    --enable-shared \
    --enable-pic \
    --host=$my_host \
	--cross-prefix=$CROSS_PREFIX \
    --sysroot=$NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot \
 
make clean
make -j8
make install
}
 
#arm64-v8a
PREFIX=./android/arm64-v8a
my_host=aarch64-linux-android
export TARGET=aarch64-linux-android
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
build_one
 
#armeabi-v7a
PREFIX=./android/armeabi-v7a
my_host=armv7a-linux-android
export TARGET=armv7a-linux-androideabi
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-