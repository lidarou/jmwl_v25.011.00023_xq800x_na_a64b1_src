#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

# Set architecture and CPU
set(BL_SYSTEM_PROCESSOR cortex-m33+nodsp)
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

# set(CONFIG_BL_SDK ON  CACHE BOOL "Add bootloader SDK config.")

#secure boot
set(AIC_HEADER_MAGIC_NUMBER 0x55aa7e7e)
set(AIC_SW_VERISON 0x0)

# serial ip version.
set(CONFIG_AIC_UART_VERSION "r1p0" CACHE STRING "AIC UART IP Version.")

# pmu ip version.
set(CONFIG_AIC_PMU_VERSION "r1p0" CACHE STRING "AIC PMU IP Version.")
add_definitions(-DCONFIG_AIC_PMU_R1P0)

# gpio ip version.
set(CONFIG_GPIO_VERSION "cnds" CACHE STRING "AIC GPIO IP Version.")

# DMA ip version.
set(CONFIG_DMA_VERSION "r1p0" CACHE STRING "AIC DMA IP Version.")

# ssp ip version.
set(CONFIG_AIC_SSP_VERSION "r1p0" CACHE STRING "AIC SSP IP Version.")
