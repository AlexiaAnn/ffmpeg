make clean
./configure --prefix=x264_install --enable-static --enable-shared --enable-debug \
--extra-cflags="-Il-smash-install/include" --extra-ldflags="-Ll-smash-install/lib"
# make -j 32
# make install