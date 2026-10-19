# General configuration options
#
# Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_DRIVER_GPO
    ${CMAKE_CURRENT_LIST_DIR}/gpo_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_GPO_VER}/gpo_phy.c
)

aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_GPO_VER}
)
