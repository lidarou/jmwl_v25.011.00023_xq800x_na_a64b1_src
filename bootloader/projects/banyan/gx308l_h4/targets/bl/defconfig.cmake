#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

#subsys
set(CONFIG_BL ON  CACHE BOOL "Add target define.")
add_definitions(-DCONFIG_BL)

set(CONFIG_DRIVER_CS ON  CACHE BOOL "Add DRIVER CLSOE SOURCE LIB.")
add_definitions(-DCONFIG_DRIVER_CS)

#module
set(CONFIG_BL_AIC_UART ON  CACHE BOOL "Add AIC UART driver.")
add_definitions(-DCONFIG_BL_AIC_UART)

#set(CONFIG_DOWNLOAD_UART ON  CACHE BOOL "Add UART download driver.")
#add_definitions(-DCONFIG_DOWNLOAD_UART)

set(CONFIG_BL_CPU_CACHE ON  CACHE BOOL "Add CPU CACHE driver.")
add_definitions(-DCONFIG_BL_CPU_CACHE)

set(CONFIG_BL_CPU_MPU ON  CACHE BOOL "Add CPU MPU driver.")
add_definitions(-DCONFIG_BL_CPU_MPU)

set(CONFIG_PMIC_I2C ON  CACHE BOOL "Add PMIC I2C driver.")
add_definitions(-DCONFIG_PMIC_I2C)

set(CONFIG_PMIC_VERSION gp201 CACHE STRING "Add pmic id")
add_definitions(-DCONFIG_PMIC_VERSION)

set(CONFIG_I2C ON  CACHE BOOL "Add I2C driver.")
add_definitions(-DCONFIG_I2C)

set(CONFIG_BOOT ON CACHE BOOL "Add boot driver.")
add_definitions(-DCONFIG_BOOT)

set(CONFIG_BOOT_MODE ON CACHE BOOL "Add boot mode.")
add_definitions(-DCONFIG_BOOT_MODE)

#set(CONFIG_MBL_BOOT ON CACHE BOOL "Add MBL boot driver.")
#add_definitions(-DCONFIG_MBL_BOOT)

#set(CONFIG_DOWNLOAD ON CACHE BOOL "Add download driver.")
#add_definitions(-DCONFIG_DOWNLOAD)

#set(CONFIG_DL_DOWNLOAD ON CACHE BOOL "Add download driver in DL stages.")
#add_definitions(-DCONFIG_DL_DOWNLOAD)

#set(CONFIG_MDL_DOWNLOAD ON CACHE BOOL "Add download driver in MDL stages.")
#add_definitions(-DCONFIG_MDL_DOWNLOAD)

set(CONFIG_EFUSE ON CACHE BOOL "Add EFUSE driver.")
add_definitions(-DCONFIG_EFUSE)

set(CONFIG_FMD ON CACHE BOOL "Add FMD driver.")
add_definitions(-DCONFIG_FMD)

set(CONFIG_SNC_QSPI ON CACHE BOOL "Add QSPI driver.")
set(CONFIG_QSPI_VER "r1p1" CACHE STRING "QSPI IP Version.")
add_definitions(-DCONFIG_SNC_QSPI)

#set(CONFIG_FMDPART_UPDATE ON CACHE BOOL "Add fmd update driver.")
#add_definitions(-DCONFIG_FMDPART_UPDATE)

set(CONFIG_CMDLINE ON CACHE BOOL "Add CMDLINE driver.")
add_definitions(-DCONFIG_CMDLINE)

#set(CONFIG_GXEM ON CACHE BOOL "Add GXEM driver.")
#add_definitions(-DCONFIG_GXEM)

set(CONFIG_PSRAM ON CACHE BOOL "Add PSRAM driver.")
set(CONFIG_PSRAM_VER "r2p0" CACHE STRING "PSRAM IP Version.")
add_definitions(-DCONFIG_PSRAM)

#set(CONFIG_SECURE_BOOT ON CACHE BOOL "Add SECURE_BOOT driver.")
#add_definitions(-DCONFIG_SECURE_BOOT)

#set(CONFIG_CRYPTO_SW_ACCELERATOR ON CACHE BOOL "Whether to enable the crypto software accelerator on supported platforms")
#add_definitions(-DCONFIG_CRYPTO_SW_ACCELERATOR)

set(CONFIG_AIC_SIGNATURE ON  CACHE BOOL "Whether to enable signature function")

set(CONFIG_TIMER ON CACHE BOOL "Add TIMER driver.")
add_definitions(-DCONFIG_TIMER)

set(CONFIG_USB ON CACHE BOOL "Add USB driver.")
add_definitions(-DCONFIG_USB)

set(CONFIG_BOOT_USB ON CACHE BOOL "Add USB driver in boot stage.")
add_definitions(-DCONFIG_BOOT_USB)

#set(CONFIG_DOWNLOAD_USB ON CACHE BOOL "Add USB driver in download stage.")
#add_definitions(-DCONFIG_DOWNLOAD_USB)

set(CONFIG_PIN ON CACHE BOOL "Add PIN driver.")
add_definitions(-DCONFIG_PIN)

set(CONFIG_SYS_DUMP ON CACHE BOOL "Add sys dump function.")
add_definitions(-DCONFIG_SYS_DUMP)

set(CONFIG_SYS_DUMP_INIT_UART_PIN ON CACHE BOOL "Add sys dump init uart pin function.")
add_definitions(-DCONFIG_SYS_DUMP_INIT_UART_PIN)

#set(CONFIG_CLK ON CACHE BOOL "Add CLK driver.")
#add_definitions(-DCONFIG_CLK)

set(CONFIG_FOTA_SUPPORT ON CACHE BOOL "Enable fota support.")
add_definitions(-DCONFIG_FOTA_SUPPORT)

set(CONFIG_HLZMA_SUPPORT ON CACHE BOOL "Enable hlzma support.")
add_definitions(-DCONFIG_HLZMA_SUPPORT)

set(CONFIG_PMIC_CLK ON CACHE BOOL "Add PMIC CLK driver.")
add_definitions(-DCONFIG_PMIC_CLK)

set(CONFIG_BCD ON CACHE BOOL "Add bcd driver.")
add_definitions(-DCONFIG_BCD)

set(CONFIG_PMIC_WDT ON CACHE BOOL "Add pmic wdt driver.")
set(CONFIG_PMIC_WDT_VERSION "r2p0" CACHE STRING "pmic wdt ip version.")
add_definitions(-DCONFIG_PMIC_WDT)

set(CONFIG_FOTA_UPGRADE ON CACHE BOOL "Add FOTA UPGRADE FLAG at bin end.")
add_definitions(-DCONFIG_FOTA_UPGRADE)

set(CONFIG_PMIC_SLP ON CACHE BOOL "Add pmic slp config")
add_definitions(-DCONFIG_PMIC_SLP)

set(CONFIG_PMU_SLP ON CACHE BOOL "Add pmu slp config")
add_definitions(-DCONFIG_PMU_SLP)

set(CONFIG_PMIC_MISC ON CACHE BOOL "Add pmic misc config")
add_definitions(-DCONFIG_PMIC_MISC)

set(CONFIG_GPIO ON CACHE BOOL "Add GPIO config")
add_definitions(-DCONFIG_GPIO)

#set(CONFIG_AUTO_DL ON CACHE BOOL "Add auto download config")
#add_definitions(-DCONFIG_AUTO_DL)

set(CONFIG_COMPRESSED_BIN ON CACHE BOOL "Add compress bin feature")
add_definitions(-DCONFIG_COMPRESSED_BIN)

set(CONFIG_BCD_DUMMY ON CACHE BOOL "Add bcd dummy driver.")
add_definitions(-DCONFIG_BCD_DUMMY)

#set(CONFIG_LOG_DECREASE ON CACHE BOOL "Decrease log.")
#add_definitions(-DCONFIG_LOG_DECREASE)

set(CONFIG_BUCK1_ON ON CACHE BOOL "buck1 sleep on.")
add_definitions(-DCONFIG_BUCK1_ON)

set(CONFIG_BYPASS_HLZMA_EB CACHE BOOL "Not disable lzma eb")
add_definitions(-DCONFIG_BYPASS_HLZMA_EB)
