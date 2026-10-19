# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/efuse_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_EFUSE_VER}/efuse_phy.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_UID_VERSION}/uid.c
    ${CMAKE_CURRENT_LIST_DIR}/at_efuse_uid.c
    ${CMAKE_CURRENT_LIST_DIR}/diag_efuse_uid.c
)

aic_library_sources_ifdef(CONFIG_DRIVER_EFUSE_DEMO efuse_demo.c)