# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/gpio_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_GPIO_VER}/gpio_phy.c
)

aic_library_sources_ifdef(
    CONFIG_SUBSYS_AP
    ${CMAKE_CURRENT_LIST_DIR}/gpio_at.c
)

aic_library_sources_ifdef(
    CONFIG_GPIO_NETLIGHT_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/sample/gpio_netlight_sample.c
)


aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_GPIO_VER}
)

