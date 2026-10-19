2024.06.24
1、demo例程使用视频文件模拟摄像头的视频流，视频文件保存在/o/demo_media/ 路径，
2、时间云存测试线程，在时间触发时，需要提供触发事件的图片，图片保存在/o/demo_media/pic/ 路径，
3、需要打开Generate FATFS image at build time， 使用python ./make.py ap menuconfig
   该选项的路径是components Options -> File System Config -> File System (FS) support -> FAT File System support
4、video_sdk运行需要的文件都保存在/o路径下。比如/o/demo_media/, device_info.json,
5、第三点还需要同时修改文件，pkg_cfg.json, fmd_partition_table.json, fs_config.h。 修改点可以和归芯SDK主线对比，
   修改的内容是为了让/o路径的磁盘空间能够存下第四点所说的若干文件。大约200KB。
6、video_sdk使用归芯SDK提供的mbedtls库，但是修改了config.h的内容，修改点和归芯SDK主线对比。
7、video_sdk提供的库文件保存在 aicos\vendors\tc-iot-video-c-sdk\libs
\ap\ 路径。
   video_sdk提供的头文件保存在 aicos\vendors\tc-iot-video-c-sdk
8、编译时，先打开第三点所说的编译选项，然后./make.py all即可。
