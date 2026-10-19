# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/outbyte.c
    ${CMAKE_CURRENT_LIST_DIR}/xuartps_hw.c
)

aic_library_sources(${SRCS})
