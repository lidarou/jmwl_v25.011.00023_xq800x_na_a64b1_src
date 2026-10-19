# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/drivers/sound/driver
)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/voice_trans.c
)

aic_library_sources(${SRCS})
