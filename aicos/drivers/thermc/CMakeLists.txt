# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/thermc_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/thermc_phy/${CONFIG_THERMC_VER}/thermc_phy.c
    ${CMAKE_CURRENT_LIST_DIR}/therm_sensor/${CONFIG_THERM_SENSOR_VER}/therm_sensor.c
)