# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_PWM_DRIVER
    ${CMAKE_CURRENT_LIST_DIR}/pwm_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/pwm_phy.c
)

aic_library_sources_ifdef(CONFIG_PWM_AT pwm_at_handle.c)