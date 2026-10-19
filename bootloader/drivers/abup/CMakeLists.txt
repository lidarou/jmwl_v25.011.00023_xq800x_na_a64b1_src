# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

target_sources(boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Src/abup_bl_main.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/abup_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/abup_hal_flash.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/abup_hal_uart.c
    ${CMAKE_CURRENT_LIST_DIR}/Wosun/src/abup_bl_flash.c
)

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/Inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/Wosun/inc>
)


target_link_libraries(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/abup_wosun_Release_20230811.a
)
