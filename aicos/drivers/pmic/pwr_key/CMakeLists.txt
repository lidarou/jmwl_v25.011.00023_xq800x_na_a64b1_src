# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_PWR_KEY_KICK_SHUTDOWN
    ${CMAKE_CURRENT_LIST_DIR}/pwr_key_shutdown.c
)

aic_library_sources_ifdef(
    CONFIG_PWR_KEY_LONG_PRESS_SHUTDOWN
    ${CMAKE_CURRENT_LIST_DIR}/pwr_key_long_press_shutdown.c
)

aic_library_sources_ifdef(
    CONFIG_PWR_KEY_GND
    ${CMAKE_CURRENT_LIST_DIR}/pwr_key_gnd.c
)

aic_library_sources_ifdef(
    CONFIG_DISABLE_CHG_IN_BOOT_LPR
    ${CMAKE_CURRENT_LIST_DIR}/disable_chg_in_boot_lpr.c
)