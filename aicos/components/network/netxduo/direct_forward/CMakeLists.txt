# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/aic_net_direct_forward.c
)

aic_library_sources(${SOURCES})

aic_include_directories( ${CMAKE_CURRENT_LIST_DIR})
