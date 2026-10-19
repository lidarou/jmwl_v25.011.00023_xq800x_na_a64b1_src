#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

# Set architecture and CPU
set(BL_SYSTEM_PROCESSOR cortex-m33)
set(AIC_ARCH_DIR arch/arm/cmstar/)
set(AIC_HWI_DIR "${CMAKE_SOURCE_DIR}/../hwi/${CMAKE_CONFIG_CHIP}/ap")
set(AIC_PINLST_DIR "${CMAKE_SOURCE_DIR}/../hwi/${CMAKE_CONFIG_CHIP}")
set(AIC_PPINLST_DIR  "${CMAKE_SOURCE_DIR}/../hwi")
set(AIC_TARGET_DIR "${CMAKE_SOURCE_DIR}/../targets/${CMAKE_CONFIG_CHIP}/${CMAKE_CONFIG_PROJ}")
set(AIC_OUT_DIR "${CMAKE_SOURCE_DIR}/../out/${CMAKE_CONFIG_PROJ}/${CMAKE_SUBSYS_CONFIG}/${SUB_TARGET}")
set(AIC_BL_BINARY_DIR "${CMAKE_BINARY_DIR}/..")
set(AIC_BL_NAME "${SUB_TARGET}")
set(AIC_PMIC_DIR "${CMAKE_SOURCE_DIR}/../hwi/${CONFIG_PMIC_VERSION}")
set(AIC_BUILD "${CMAKE_SOURCE_DIR}/../build")
add_definitions(-DCONFIG_ARMV8_M_DSP)
set(AIC_SIGN_KEY_DIR "${AIC_TARGET_DIR}/key")
# set(CONFIG_BL_SDK ON  CACHE BOOL "Add bootloader SDK config.")

#secure boot
set(AIC_HEADER_MAGIC_NUMBER 0x55aa7e7e)
set(AIC_SW_VERISON 0x0)

# serial ip version.
set(CONFIG_AIC_UART_VERSION "r2p0" CACHE STRING "AIC UART IP Version.")

# pmu ip version.
set(CONFIG_AIC_PMU_VERSION "r2p0" CACHE STRING "AIC PMU IP Version.")
add_definitions(-DCONFIG_AIC_PMU_R2P0)

# gpio ip version.
set(CONFIG_GPIO_VERSION "r1p0" CACHE STRING "AIC GPIO IP Version.")

# DMA ip version.
set(CONFIG_DMA_VERSION "r2p0" CACHE STRING "AIC DMA IP Version.")

# ssp ip version.
set(CONFIG_AIC_SSP_VERSION "r1p1" CACHE STRING "AIC SSP IP Version.")

if ((${CMAKE_SECURE_BOARD} STREQUAL "SEC"))
#enable secure boot
set(CONFIG_SECURE_BOOT ON CACHE BOOL "Add SECURE_BOOT driver.")
add_definitions(-DCONFIG_SECURE_BOOT)

set(CONFIG_CRYPTO_SW_ACCELERATOR ON CACHE BOOL "Whether to enable the crypto software accelerator on supported platforms")
add_definitions(-DCONFIG_CRYPTO_SW_ACCELERATOR)
endif()
