HOME=/home/alexia/ffmpeg
ANDROID_NDK=$HOME/android-ndk-r21e
make clean
function build_x264()
{
PREFIX=$(pwd)/x264-android/$ANDROID_ABI
SYSROOT=$ANDROID_NDK/platforms/$ANDROID_API/$ANDROID_ARCH
TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/$HOST
CROSS_PREFIX=$TOOLCHAIN/$CROSS_COMPILE

echo $PREFIX
cd x264-master
echo "Compiling x264 for $ANDROID_ABI"
./configure \
    --prefix=$PREFIX \
    --disable-asm \
    --enable-static \
    --enable-shared \
    --enable-pic \
    --host=$HOST \
    --cross-prefix=$CROSS_PREFIX \
    --sysroot=$SYSROOT \
    
make clean
make -j4
make install
echo "The Compilation of x264 for $ANDROID_ABI is completed"
}

# armeabi-v7a
ANDROID_ABI=armeabi-v7a
ANDROID_API=android-14
ANDROID_ARCH=arch-arm
ANDROID_EABI=arm-linux-androideabi-4.9

HOST=arm-linux-androideabi
CROSS_COMPILE=arm-linux-androideabi-
build_x264

# arm64-v8a
ANDROID_ABI=arm64-v8a
ANDROID_API=android-21
ANDROID_ARCH=arch-arm64
ANDROID_EABI=aarch64-linux-android-4.9
HOST=aarch64-linux-android
CROSS_COMPILE=aarch64-linux-android-
build_x264