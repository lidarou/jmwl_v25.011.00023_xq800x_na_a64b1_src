# HALU: Hardware Abstraction Layer and Utilities
include_directories(internal_inc)
#include_directories(${IOT_VIDEO_SDK_SOURCE_DIR}/ext_libs/mbedtls/include)
include_directories(${IOT_VIDEO_SDK_SOURCE_DIR}/../crypto/mbedtls-3.3.0/include)

message("-------ROCKYSHEN os is ${OS}")
# source file
file(GLOB src_hal ${CMAKE_CURRENT_SOURCE_DIR}/os/${OS}/*.c)
file(GLOB src_tls ${CMAKE_CURRENT_SOURCE_DIR}/tls/*.c)
file(GLOB src_utils ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.c)

set(LINKAGE ${QCLOUD_VIDEO_COMPILE_LIBS_TYPE})


#set(target iot_halu)
message("--------halu target is : ${target}")
message("--------halu LIBRARY_OUTPUT_PATH is : ${LIBRARY_OUTPUT_PATH}")
aic_library_named(iot_halu)

# output path
set(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})

message(STATUS ${target} " build " ${QCLOUD_VIDEO_COMPILE_LIBS_TYPE} " Library")
if(${QCLOUD_VIDEO_COMPILE_LIBS_TYPE} STREQUAL "SHARED")
	file(REMOVE ${LIBRARY_OUTPUT_PATH}/lib${target}.a)
else()
	file(REMOVE ${LIBRARY_OUTPUT_PATH}/lib${target}.so)
endif()

#add_library(${target} ${LINKAGE} ${src_hal} ${src_tls} ${src_utils})
aic_library_sources(${src_hal} ${src_tls} ${src_utils})


