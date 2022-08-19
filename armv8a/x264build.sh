basepath=$(pwd)

echo ${basepath}

cd ${basepath}/x264-master
pwd


./configure --prefix=${basepath}/x264_install --enable-static --disable-thread
make 
make install