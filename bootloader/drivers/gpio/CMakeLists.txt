# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

target_sources(boot_driver
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/gpio_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_GPIO_VERSION}/gpio_phy.c
)

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${CONFIG_GPIO_VERSION}>
)