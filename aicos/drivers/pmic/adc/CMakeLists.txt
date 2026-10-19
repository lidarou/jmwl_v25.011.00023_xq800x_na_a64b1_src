# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/adc_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_ADC_VER}/adc_phy.c
)

aic_library_sources_ifdef(CONFIG_ADC_BLT_AT_CMD adc_at_handle.c)
aic_library_sources_ifdef(CONFIG_DRIVER_ADC_DEMO adc_demo.c)