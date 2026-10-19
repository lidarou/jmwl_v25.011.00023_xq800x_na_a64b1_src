# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_include_directories(${CMAKE_CURRENT_LIST_DIR})

aic_library_sources_ifdef(
    CONFIG_GX316_CSI
    ${CMAKE_CURRENT_LIST_DIR}/sensor_private.c
)

aic_library_sources_ifdef(
    CONFIG_CAMERA_SENSOR_GC032A
    ${CMAKE_CURRENT_LIST_DIR}/sensor_gc032a.c
)

aic_library_sources_ifdef(
    CONFIG_CAMERA_SENSOR_BF20A6
    ${CMAKE_CURRENT_LIST_DIR}/sensor_bf20a6.c
)

aic_library_sources_ifdef(
    CONFIG_CAMERA_SENSOR_BF30A2
    ${CMAKE_CURRENT_LIST_DIR}/sensor_bf30a2.c
)

aic_library_sources_ifdef(
    CONFIG_CAMERA_SENSOR_BF3901UCS
    ${CMAKE_CURRENT_LIST_DIR}/sensor_bf3901ucs.c
)

aic_library_sources_ifdef(
    CONFIG_CAMERA_SENSOR_GC6133
    ${CMAKE_CURRENT_LIST_DIR}/sensor_gc6133.c
)
