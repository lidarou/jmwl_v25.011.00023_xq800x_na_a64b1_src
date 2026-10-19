# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/codec_wav.c
)

aic_library_sources_ifdef(
    CONFIG_WAV_SUPPORT
    ${SRCS}
)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/mm/audio
    ${AIC_SOURCE_ROOT}/mm/codec
)