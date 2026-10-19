# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

target_sources(boot_driver
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/dma_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DMA_VERSION}/dma_phy.c
)
