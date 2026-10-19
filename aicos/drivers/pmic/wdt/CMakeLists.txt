# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/pmic_wdt_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_PMIC_WDT_VER}/pmic_wdt_phy.c
)