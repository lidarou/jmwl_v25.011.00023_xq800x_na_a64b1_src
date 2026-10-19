# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/cortexm7_crt0.s
    ${CMAKE_CURRENT_LIST_DIR}/cache.s
    ${CMAKE_CURRENT_LIST_DIR}/mpu.c
    ${CMAKE_CURRENT_LIST_DIR}/cputime_cortexm.c
    ${CMAKE_CURRENT_LIST_DIR}/tx_simulator_startup.s
    ${CMAKE_CURRENT_LIST_DIR}/tx_initialize_low_level.s
    )

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/os/azure-rtos/threadx/ports/cortex_m7/gnu/inc
)

aic_library_sources(${SRCS})
