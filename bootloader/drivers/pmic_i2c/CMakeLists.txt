# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

target_sources(boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/pmic_i2c_phy.c
    ${CMAKE_CURRENT_LIST_DIR}/pmic_i2c_core.c
    ${CMAKE_CURRENT_LIST_DIR}/pmic_i2c.c
)

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${AIC_PMIC_DIR}>
)