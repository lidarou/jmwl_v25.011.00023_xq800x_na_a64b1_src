# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

FILE(GLOB SRC_LIST_C ${CMAKE_CURRENT_LIST_DIR}/src/*.c)

aic_library_sources(
    ${SRC_LIST_C}
)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/mm/resource
    ${AIC_SOURCE_ROOT}/mm/codec
    ${AIC_SOURCE_ROOT}/mm/audio/nv
    ${AIC_SOURCE_ROOT}/drivers/sound/adsp
    ${AIC_SOURCE_ROOT}/mm/audio/vendor/tts_yt
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/src
)


