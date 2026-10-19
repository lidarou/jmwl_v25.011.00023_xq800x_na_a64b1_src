# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
target_sources_ifdef(CONFIG_GRAPHICS boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/core.c
    ${CMAKE_CURRENT_LIST_DIR}/lcdc/lcdc_drv.c
    ${CMAKE_CURRENT_LIST_DIR}/panel/${CONFIG_PANEL_NAME}/panel.c
)

target_sources_ifdef(CONFIG_BACKLIGHT_KPLED boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/backlight/backlight_kpled.c
)

target_sources_ifdef(CONFIG_BACKLIGHT_RGBW boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/backlight/backlight_rgbw.c
)

target_include_directories(boot_driver
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${AIC_PMIC_DIR}>
)