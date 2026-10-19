# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/inc>
)

target_sources(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/gigadevice.c
        ${CMAKE_CURRENT_SOURCE_DIR}/winbond.c
        ${CMAKE_CURRENT_SOURCE_DIR}/xmc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/puya.c
)
