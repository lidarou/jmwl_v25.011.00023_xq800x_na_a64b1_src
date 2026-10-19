# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/pmic_efuse_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_PMIC_EFUSE_VER}/pmic_efuse_phy.c
#    ${CMAKE_CURRENT_LIST_DIR}/pmic_efuse_at.c
)

aic_library_sources_ifdef(CONFIG_DRIVER_PMIC_EFUSE_DEMO pmic_efuse_demo.c)