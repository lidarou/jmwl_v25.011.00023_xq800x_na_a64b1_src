# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(CONFIG_DRIVER_I2C
    ${CMAKE_CURRENT_LIST_DIR}/i2c_phy.c
    ${CMAKE_CURRENT_LIST_DIR}/i2c_core.c
)

aic_library_sources_ifdef(CONFIG_DRIVER_I2C_TEST
    ${CMAKE_CURRENT_LIST_DIR}/i2c_test.c
)

aic_include_directories(
    ./inc
)