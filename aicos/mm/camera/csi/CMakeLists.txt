# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_GX316_CSI
    ${CMAKE_CURRENT_LIST_DIR}/csi_phy.c
    ${CMAKE_CURRENT_LIST_DIR}/csi_hal.c
)

aic_include_directories_ifdef(CONFIG_GX316_CSI
    ${CMAKE_CURRENT_LIST_DIR}
)
