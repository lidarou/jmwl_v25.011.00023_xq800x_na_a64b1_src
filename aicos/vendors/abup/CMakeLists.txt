# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_named(aic_abup)

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_device.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_display.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_flash.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_mcu.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_md5.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_net.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_stdlib.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_thread.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_timer.c
    ${CMAKE_CURRENT_LIST_DIR}/hal/src/abup_file.c
)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/app/include
    ${CMAKE_CURRENT_LIST_DIR}/hal/include
)

aic_library_import(libaic_abup
    ${CMAKE_CURRENT_LIST_DIR}/lib/abup_fota_20230811.a
)

