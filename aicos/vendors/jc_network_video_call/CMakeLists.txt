# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_named(aic_vendors)

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/jc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/mme_audio.c
    ${CMAKE_CURRENT_LIST_DIR}/src/mme_video.c
    #${CMAKE_CURRENT_LIST_DIR}/src/rtptools.c
)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

aic_library_import(libjrtc0
    ${CMAKE_CURRENT_LIST_DIR}/lib/libjrtc0.a
)
