# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
aic_library_sources_ifdef(
    CONFIG_BCD
    ${CMAKE_CURRENT_LIST_DIR}/bcd_core.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_BCD_IP}/bcd_phy.c
)

aic_library_sources_ifdef(
    CONFIG_BCD_DEMO
    ${CMAKE_CURRENT_LIST_DIR}/bcd_demo.c
)