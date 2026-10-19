# General configuration options
#
# Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/aic_d2d_main.c
)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
)
