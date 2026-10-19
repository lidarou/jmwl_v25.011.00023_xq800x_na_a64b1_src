# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_net_usb_semi_tether.c
)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
)