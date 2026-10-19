# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/wdt_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_WDT_VER}/wdt_phy.c
)