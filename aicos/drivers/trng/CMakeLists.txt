# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(CONFIG_AIC_TRNG
    ${CMAKE_CURRENT_LIST_DIR}/trng_phy.c
    ${CMAKE_CURRENT_LIST_DIR}/trng_hal.c
)
