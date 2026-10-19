# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_IIS_VERSION}
)

aic_library_sources(
#    ${CMAKE_CURRENT_LIST_DIR}/iis.c
    ${CMAKE_CURRENT_LIST_DIR}/iis_phy.c
)
