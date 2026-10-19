# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/rtc-lib.c
    ${CMAKE_CURRENT_LIST_DIR}/rtc_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRV_RTC_PMIC_VER}/rtc_phy.c
)
