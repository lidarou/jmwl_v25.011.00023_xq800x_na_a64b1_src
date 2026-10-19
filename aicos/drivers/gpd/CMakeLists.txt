# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/gpd_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_GPD_VER}/gpd_phy.c
)

aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_GPD_VER}
)

