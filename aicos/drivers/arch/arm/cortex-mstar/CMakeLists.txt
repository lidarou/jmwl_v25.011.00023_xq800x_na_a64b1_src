# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/mstar_crt0.s
    ${CMAKE_CURRENT_LIST_DIR}/startup.s
    ${CMAKE_CURRENT_LIST_DIR}/irq.c
    ${CMAKE_CURRENT_LIST_DIR}/cache.c
    ${CMAKE_CURRENT_LIST_DIR}/tzc.c
    ${CMAKE_CURRENT_LIST_DIR}/mpu.c
    )

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/os/azure-rtos/threadx/ports/cortex_mstar/gnu/inc
)

aic_library_sources(${SRCS})
