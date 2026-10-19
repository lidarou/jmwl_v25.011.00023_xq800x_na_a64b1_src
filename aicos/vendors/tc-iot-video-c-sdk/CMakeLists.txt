# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.6)
aic_library_named(tc_iot)

message("enter rockyshen iv sdk")

set(PLATFORM                         "GX316" CACHE STRING INTERNAL)
set(QCLOUD_VIDEO_COMPILE_SAMPLE_ONLY "OFF" CACHE STRING INTERNAL)
set(QCLOUD_VIDEO_COMPILE_SAMPLE      "OFF" )
set(QCLOUD_VIDEO_COMPILE_CSDK_LIB    "ON" CACHE STRING INTERNAL)
set(QCLOUD_VIDEO_COMPILE_LIBS_TYPE   "STATIC" CACHE STRING INTERNAL)

set(QCLOUD_VIDEO_COMPILE_EXTERNAL_LIBS_NAME "all" CACHE STRING INTERNAL)

set(QCLOUD_VIDEO_DEMO_NAME           "demo" CACHE STRING INTERNAL)
set(ARCH                             "arm" CACHE STRING INTERNAL)
set(OS                               "aicos" CACHE STRING INTERNAL)
set(COMPILE_FLAG                     "-fPIC -std=gnu99" CACHE STRING INTERNAL)
set(BUILD_TYPE                       "debug" CACHE STRING INTERNAL)
set(IOT_VIDEO_PROJECT_DIR            ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING INTERNAL)
#set(CMAKE_C_COMPILER  ${TOOL_CHAIN_PATH}/bin/${COMPILE_TOOLS}gcc)
#set(CMAKE_CXX_COMPILER  ${TOOL_CHAIN_PATH}/bin/${COMPILE_TOOLS}g++)

message("enter rockyshen var set")
# 项目信息
project(qcloud_iot_video_c_sdk)

######################CONFIG BEGIN####################################
# 编译类型: release/debug
set(BUILD_TYPE "debug" CACHE STRING INTERNAL)

set(QCLOUD_VIDEO_COMPILE_LIBS_TYPE "STATIC" CACHE STRING INTERNAL)
# 临时屏蔽
#set(QCLOUD_VIDEO_COMPILE_EXTERNAL_LIBS_NAME "" CACHE STRING INTERNAL)
set(QCLOUD_VIDEO_COMPILE_CSDK_HAL "ON" CACHE STRING INTERNAL)
set(QCLOUD_VIDEO_COMPILE_CSDK_LIB "ON" CACHE STRING INTERNAL)
set(QCLOUD_VIDEO_COMPILE_SAMPLE_ONLY "OFF" CACHE STRING INTERNAL)

# 编译工具链

set(CMAKE_FIND_ROOT_PATH ${TOOL_CHAIN_PATH})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_FLAG}")

# 设置SDK录地址
set(IOT_VIDEO_SDK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置IoT Video SDK的版本号
set(IOT_VIDEO_SDK_VERSION "\"2.4.45\"")

# 是否使能日志着色
set(FEATURE_LOG_COLOR_ENABLED ON)

# 是否使能P2P视频传输功能
set(FEATURE_AVT_P2P_ENABLED ON)

# 是否图片流设备，打开则视频流走mjpg模式而不是h264/265模式
set(FEATURE_AVT_MJPG_ENABLED OFF)

# 是否打开avt视频传输的局域网功能
set(FEATURE_AVT_LAN_ENABLED OFF)

# 是否打开avt的TCP通信功能
set(FEATURE_AVT_TCP_ENABLED OFF)

# 视频是否添加SEI数据
set(FEATURE_AVT_SEI_ENABLE OFF)

# 是否使能直播AAC转码
set(FEATURE_AVT_AACENC_ENABLED OFF)

# 是否使能云存功能
set(FEATURE_CLOUD_STORAGE_ENABLED ON)

# 云存使用https-ts协议, 该协议只支持AAC,不支持pcm.g711a,g711u, 故aac转码跟随云存开关选择是否开启
# 是否使能云存AAC转码
set(FEATURE_CS_AACENC_ENABLED ${FEATURE_CLOUD_STORAGE_ENABLED})

# 云存使用ts格式
set(FEATURE_CS_FORMAT_TS ON)

# 云存使用fmp4格式
set(FEATURE_CS_FORMAT_FMP4 ON)

# 云存是否使能HTTPS
set(FEATURE_CS_USE_HTTPS ON)

# 云存调试选项，在用户送流时导出视频
set(FEATURE_CS_DUMP_AT_WRITE OFF)

# 云存调试选项，在上传时导出视频
set(FEATURE_CS_DUMP_AT_UPLOAD OFF)

# 是否内部使用MQTT进行通信
set(FEATURE_DM_CONFIG_USE_MQTT ON)

# 是否使能OTA固件升级功能
set(FEATURE_OTA_COMM_ENABLED ON)

# 是否使能物模型事件上报
set(FEATURE_DM_EVENT_POST_ENABLED ON)

# 是否使能物模型行为功能
set(FEATURE_DM_ACTION_ENABLED ON)

# 是否使能设备动态注册
set(FEATURE_SYS_DEV_DYN_REG_ENABLED ON)
 
# 是否打开日志上报云端功能
set(FEATURE_SYS_LOG_UPLOAD_ENABLED OFF)

# 是否开启voip微信语音通话功能（大部分平台voip团队没有适配，默认关闭）
set(FEATURE_AVT_VOIP_ENABLE ON)

# 是否为天视通设备，涉及域名IP相关
set(FEATURE_TOPSEE_DEVICE OFF)

# 图片流设备版本号需要以m结尾
if(${FEATURE_AVT_MJPG_ENABLED} STREQUAL "ON")
    set(IOT_VIDEO_SDK_VERSION "${IOT_VIDEO_SDK_VERSION}\"m\"")
endif()
######################CONFIG END######################################
# set(CMAKE_VERBOSE_MAKEFILE ON)

# 编译工具链检测
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -fPIC ${CMAKE_CXX_FLAGS}")
endif(CMAKE_COMPILER_IS_GNUCXX)

if(${PLATFORM} STREQUAL "x86")
  set(CMAKE_EXE_LINKER_FLAGS " -no-pie")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    if(${BUILD_TYPE} STREQUAL  "release")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Os") # 编译选项
    elseif(${BUILD_TYPE} STREQUAL  "debug")
        message("buid type is debug--")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g") # 编译选项
    endif()
endif()

if(CMAKE_COMPILER_IS_CLANG)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_USE_32BIT_TIME_T)
    if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W2" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W2")
    endif()
endif()

# 解析配置
if(${BUILD_TYPE} STREQUAL  "release")
    option(IOT_DEBUG "Enable IOT_DEBUG" OFF)
elseif(${BUILD_TYPE} STREQUAL  "debug")
    option(IOT_DEBUG "Enable IOT_DEBUG" OFF)
else()
    message(FATAL_ERROR "INVAILD BUILD_TYPE:${BUILD_TYPE}!")
endif()

option(AVT_P2P_ENABLED  "Enable AVT AND P2P" ${FEATURE_AVT_P2P_ENABLED})
option(AVT_MJPG_ENABLED "Enable mjpg stream" ${FEATURE_AVT_MJPG_ENABLED})
option(LOG_COLOR_ENABLED "Enable log color"  ${FEATURE_LOG_COLOR_ENABLED})
option(AVT_LAN_ENABLED  "Enable lan network" ${FEATURE_AVT_LAN_ENABLED})
option(AVT_SEI_ENABLED "Enable sei" ${FEATURE_AVT_SEI_ENABLE})
option(AVT_TCP_ENABLED  "Enable tcp communication" ${FEATURE_AVT_TCP_ENABLED})
option(CLOUD_STORAGE_ENABLED "Enable CLOUD_STORAGE" ${FEATURE_CLOUD_STORAGE_ENABLED})
option(OTA_COMM_ENABLED "Enable OTA_COMM" ${FEATURE_OTA_COMM_ENABLED})
option(SYS_DEV_DYN_REG_ENABLED  "dynamic register enable" ${FEATURE_SYS_DEV_DYN_REG_ENABLED})
option(DM_USE_MQTT_ENABLE "Enable dm use mqtt" ${FEATURE_DM_CONFIG_USE_MQTT})
option(AVT_VOIP_ENABLE "Enable avt use voip" ${FEATURE_AVT_VOIP_ENABLE})

#直播时flv协议本身就支持pcm,g711a,g711u,不需要转码
#if(${FEATURE_AVT_P2P_ENABLED} STREQUAL "ON")
#	option(AVT_AACENC_ENABLED "Enable live AAC ENCODE" ${FEATURE_AVT_AACENC_ENABLED})
#else()
#	option(AVT_AACENC_ENABLED "Enable live AAC ENCODE" OFF)
#endif()

# if(${FEATURE_CLOUD_STORAGE_ENABLED} STREQUAL "OFF")
# 	option(CS_AACENC_ENABLED "Diasable cloud storage AAC ENCODE" OFF)
#     option(CS_USE_HTTPS "Diasable cloud storage use https" OFF)
#     option(CS_DUMP_AT_WRITE "Diasable cloud storage debug dump when write" OFF)
#     option(CS_DUMP_AT_UPLOAD "Diasable cloud storage debug dump when upload" OFF)
# else()
#     option(CS_FORMAT_TS "Enable cloud storage ts format" ${FEATURE_CS_FORMAT_TS})
#     option(CS_FORMAT_FMP4 "Enable cloud storage fmp4 format" ${FEATURE_CS_FORMAT_FMP4})
# 	option(CS_AACENC_ENABLED "Enable cloud storage AAC ENCODE" ${FEATURE_CS_AACENC_ENABLED})
#     option(CS_USE_HTTPS "Enable cloud storage use https" ${FEATURE_CS_USE_HTTPS})
#     option(CS_DUMP_AT_WRITE "Enable cloud storage debug dump when write" ${FEATURE_CS_DUMP_AT_WRITE})
#     option(CS_DUMP_AT_UPLOAD "Enable cloud storage debug dump when upload" ${FEATURE_CS_DUMP_AT_UPLOAD})
# endif()


# 生成iv_config.h配置文件
# configure_file (
#         "${IOT_VIDEO_SDK_SOURCE_DIR}/tools/cmake_scripts/config.h.in"
#         "${IOT_VIDEO_SDK_SOURCE_DIR}/include/iv_config.h"
#         @ONLY
# )

# 公共头文件目录，SDK及sample都会依赖
aic_include_directories(${IOT_VIDEO_SDK_SOURCE_DIR}/include)
aic_include_directories(${IOT_VIDEO_SDK_SOURCE_DIR}/include/explorer_inc)
aic_include_directories(${IOT_VIDEO_SDK_SOURCE_DIR}/halu/include)


# if ("${PLATFORM}" STREQUAL "ANDROID")
#     include_directories(${TOOL_CHAIN_PATH}/sysroot/usr/include)
# 	if ("${ANDROID_ABI}" STREQUAL "x86_64")
#     	include_directories(${TOOL_CHAIN_PATH}/sysroot/usr/include/x86_64-linux-android)
# 	elseif ("${ANDROID_ABI}" STREQUAL "x86")
# 		include_directories(${TOOL_CHAIN_PATH}/sysroot/usr/include/i686-linux-android)
# 	elseif ("${ANDROID_ABI}" STREQUAL "arm64-v8a")
# 		include_directories(${TOOL_CHAIN_PATH}/sysroot/usr/include/aarch64-linux-android)
#     elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
# 		include_directories(${TOOL_CHAIN_PATH}/sysroot/usr/include/arm-linux-androideabi)
#     endif()
#     set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/${PLATFORM}/${BUILD_TYPE}/${ARCH})
#     #set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/${PLATFORM}/${ARCH}/bin/)
# else()
#     set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/${PLATFORM}/${BUILD_TYPE}/lib)
#     set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/${PLATFORM}/${BUILD_TYPE}/bin/)
# endif()

# # TODO：考虑在platform.json中增加一个linkerFlags来添加平台链接相关参数
# if(${PLATFORM} MATCHES "yunyi")
# 	set(stdcpp_flag "-static-libgcc;-static-libstdc++")
# elseif(${PLATFORM} MATCHES "FH8852_JCO")
#     set(stdcpp_flag -lstdc++ -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4)
# else()
#     set(stdcpp_flag -lstdc++)
# endif()

# 添加源代码目录




# if (${QCLOUD_VIDEO_COMPILE_CSDK_LIB} STREQUAL "ON")
# 	message(STATUS "Config core sdk src Start")
# 	add_subdirectory(${IOT_VIDEO_SDK_SOURCE_DIR}/core)
# 	message(STATUS "Config core sdk src Done")
# endif()

# if(NOT ${QCLOUD_VIDEO_COMPILE_EXTERNAL_LIBS_NAME} STREQUAL "NULL")
# 	message(STATUS "Config external_libs rockyshen" ${QCLOUD_VIDEO_COMPILE_EXTERNAL_LIBS_NAME} " Start")
# 	add_subdirectory(${IOT_VIDEO_SDK_SOURCE_DIR}/ext_libs)
# 	message(STATUS "Config external_libs rockyshen-->" ${QCLOUD_VIDEO_COMPILE_EXTERNAL_LIBS_NAME} " Done")
# endif()


# if (${QCLOUD_VIDEO_COMPILE_CSDK_HAL} STREQUAL "ON")
# 	message(STATUS "Config HAL src Start")
# 	add_subdirectory(${IOT_VIDEO_SDK_SOURCE_DIR}/halu)
# 	message(STATUS "Config HAL src Done")
# endif()


message("++++++++++++++++rockyshen compile samples! 001----")
if(${QCLOUD_VIDEO_COMPILE_SAMPLE} STREQUAL "ON")
	message(STATUS  "compile samples!")
    message("*************rockyshen compile samples! 002 =====")
    add_subdirectory(${IOT_VIDEO_SDK_SOURCE_DIR}/samples)
endif()

aic_library_import(event
    ${CMAKE_CURRENT_LIST_DIR}/libs/libevent.a
)
aic_library_import(faac
    ${CMAKE_CURRENT_LIST_DIR}/libs/libfaac.a
)
aic_library_import(flv
    ${CMAKE_CURRENT_LIST_DIR}/libs/libflv.a
)
aic_library_import(iot_explorer
    ${CMAKE_CURRENT_LIST_DIR}/libs/libiot_explorer.a
)
aic_library_import(iot_halu
    ${CMAKE_CURRENT_LIST_DIR}/libs/libiot_halu.a
)
aic_library_import(iot_video
    ${CMAKE_CURRENT_LIST_DIR}/libs/libiot_video.a
)
aic_library_import(iot_xp2p
    ${CMAKE_CURRENT_LIST_DIR}/libs/libiot_xp2p.a
)
aic_library_import(mpeg
    ${CMAKE_CURRENT_LIST_DIR}/libs/libmpeg.a
)
aic_library_import(cmov
    ${CMAKE_CURRENT_LIST_DIR}/libs/libtcmov.a
)
aic_library_import(wxcloudvoip_client
    ${CMAKE_CURRENT_LIST_DIR}/libs/libwxcloudvoip_client.a  
)