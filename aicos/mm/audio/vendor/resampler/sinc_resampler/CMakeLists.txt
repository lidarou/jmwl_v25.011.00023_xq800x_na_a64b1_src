# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_named(resampler)

#aic_compile_options(-fno-exceptions)
aic_library_cc_option(-fno-exceptions -O3)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/AudioResamplerSinc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/resampler_interface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/adpter/sincrsm_adpter.c
)



aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/adpter
    ${CMAKE_CURRENT_LIST_DIR}/..
 ${CMAKE_CURRENT_LIST_DIR}/../../..
)

aic_library_sources(${SRCS})