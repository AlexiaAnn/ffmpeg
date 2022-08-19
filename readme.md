adb 
查看目标apk进程号adb shell "ps | grep FFmpegUnity"
查看指定进程日志adb shell "logcat|grep 23365"
查看手机芯片架构 adb shell getprop ro.product.cpu.abi
linux
对一个文件夹内所有的文件赋予读写权限:chmod -R 777 filename/，赋予权限后，可以进行编译，否则会报permisson error
Makefile:2: config.mak: No such file or directory是因为./configure未正常运行的情况下执行了make命令造成的
同时存在静态链接和动态链接，gcc会优先链接动态文件
ffmpeg
查看帧数 ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 input.mp4
同步目录：playgame sources
code c++
plugins FFmpeg2 dll so
E:\Work\Unity\Assets\Source\Aones\Misc FFmpegDll