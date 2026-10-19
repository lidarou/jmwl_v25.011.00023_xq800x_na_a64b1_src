# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/codec_amr.c
)

aic_library_sources_ifdef(
    CONFIG_AMR_SUPPORT
    ${SRCS}
)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/mm/audio
    ${AIC_SOURCE_ROOT}/mm/codec
)

aic_library_import(amrwbcodec
	${CMAKE_CURRENT_LIST_DIR}/${CONFIG_AUDIO_LIB_TYPE}/libamrwb.a)

aic_library_import(amrnbcodec
        ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_AUDIO_LIB_TYPE}/libamrnb.a)
