# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

target_sources(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/ssp.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ssp_parse.c
)

target_sources(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_AIC_SSP_VERSION}/sspcore.c
)

target_include_directories(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_AIC_SSP_VERSION}
)