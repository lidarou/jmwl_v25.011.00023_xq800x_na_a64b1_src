# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(CONFIG_CSI_SUPPORT csi)
add_subdirectory_ifdef(CONFIG_JPEG_SUPPORT jpeg)
add_subdirectory_ifdef(CONFIG_CAMERA_SENSOR_SUPPORT camera_sensor)

aic_library_sources_ifdef(CONFIG_CAMERA_SUPPORT
        ${CMAKE_CURRENT_LIST_DIR}/camera_hal.c
        ${CMAKE_CURRENT_LIST_DIR}/camera_fb.c
        ${CMAKE_CURRENT_LIST_DIR}/format_convert.c
        ${CMAKE_CURRENT_LIST_DIR}/camera_sensor.c
)

aic_library_sources_ifdef(CONFIG_CAMERA_AT_CMD_SUPPORT
        ${CMAKE_CURRENT_LIST_DIR}/aic_camera_at_handle.c
)

aic_include_directories(${CMAKE_CURRENT_LIST_DIR})
