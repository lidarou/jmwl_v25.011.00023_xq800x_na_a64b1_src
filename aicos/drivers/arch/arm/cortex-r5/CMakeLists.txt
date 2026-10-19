# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/l1c.c
    ${CMAKE_CURRENT_LIST_DIR}/cache.c
    ${CMAKE_CURRENT_LIST_DIR}/mpu.c
    ${CMAKE_CURRENT_LIST_DIR}/exception.c
    ${CMAKE_CURRENT_LIST_DIR}/pmu.c
    ${CMAKE_CURRENT_LIST_DIR}/pmu_demo.c
    ${CMAKE_CURRENT_LIST_DIR}/pl192.c
    ${CMAKE_CURRENT_LIST_DIR}/tzc.c
    ${CMAKE_CURRENT_LIST_DIR}/isr_wrapper.S
    ${CMAKE_CURRENT_LIST_DIR}/irq.c
    ${CMAKE_CURRENT_LIST_DIR}/tcm.c
    ${CMAKE_CURRENT_LIST_DIR}/cpu_pc_samp.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_COMPILER_NAME}/startup.S
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_COMPILER_NAME}/crt0.S
)

if(CONFIG_OS_SYSTEM STREQUAL "azure-rtos")
    list(APPEND SRCS ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_COMPILER_NAME}/tx_initialize_low_level.S)
endif()

if (NOT "${COMPILER}" STREQUAL "clang")
    list(APPEND SRCS ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_COMPILER_NAME}/profile_counter.S)
endif()

aic_library_sources(${SRCS})

if ("${COMPILER}" STREQUAL "clang")
    set(CMAKE_LINKER_START_O
        "${AIC_BUILD_ROOT}/drivers/CMakeFiles/drivers.dir/arch/arm/cortex-r5/${CONFIG_COMPILER_NAME}/startup.o"
        CACHE STRING "")
    #
    # Why use target_ld_options instead of aic_ld_options!!!???
    # It is used to fix init* section be optimized bug. So we need add
    # another link cmd, there need add all the link flags. And all the
    # flags are set by aic_ld_options.
    # The new static name will be libXXX.a(*.o), means that the drivers
    # lib obj will include startup.o, so there do not need add startup.o
    # into link flags again(use aic_ld_options api).
    #
    #aic_ld_options(${CMAKE_LINKER_START_O})
    target_ld_options(aic_interface INTERFACE ${CMAKE_LINKER_START_O})
endif()
