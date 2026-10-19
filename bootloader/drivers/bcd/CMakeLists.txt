# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
target_sources_ifdef(CONFIG_BCD boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/bcd_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/r1p0/bcd_phy.c
)

target_include_directories(boot_driver
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${AIC_PMIC_DIR}>
)