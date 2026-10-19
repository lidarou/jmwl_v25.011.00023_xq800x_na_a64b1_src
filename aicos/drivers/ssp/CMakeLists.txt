# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_include_directories(${CONFIG_SSP_DRIVER_VER})

aic_library_sources_ifdef(CONFIG_SSP_DRIVER
    ${CMAKE_CURRENT_LIST_DIR}/ssp.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_SSP_DRIVER_VER}/sspcore.c
)
