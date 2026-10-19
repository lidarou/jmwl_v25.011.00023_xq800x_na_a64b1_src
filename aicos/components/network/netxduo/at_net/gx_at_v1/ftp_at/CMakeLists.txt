# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_ftp_at_process.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_ftp_at_extended.c
)

aic_library_sources(${SOURCES})
