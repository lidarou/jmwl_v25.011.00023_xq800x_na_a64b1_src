# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/gpio_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRV_GPIO_PMIC_VER}/gpio_phy.c
)
