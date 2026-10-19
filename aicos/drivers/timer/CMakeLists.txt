# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/timer_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_TIMER_VER}/timer_phy.c
)
