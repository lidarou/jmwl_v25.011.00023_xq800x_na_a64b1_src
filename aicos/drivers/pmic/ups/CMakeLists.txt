# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/ups_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRV_UPS_VER}/ups_phy.c
)
