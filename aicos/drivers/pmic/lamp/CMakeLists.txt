# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_DRIVER_LAMP
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_LAMP_VER}/lamp_rgbw_drv.c
    ${CMAKE_CURRENT_LIST_DIR}/lamp_hal.c
)

aic_library_sources_ifdef(CONFIG_LED_BLT_AT_CMD lamp_led_at_handle.c)
aic_library_sources_ifdef(CONFIG_DRIVER_LAMP_DEMO lamp_demo.c)