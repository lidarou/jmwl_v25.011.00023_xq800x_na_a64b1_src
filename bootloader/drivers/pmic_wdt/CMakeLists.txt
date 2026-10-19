#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_sources_ifdef(CONFIG_PMIC_WDT boot_driver
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_PMIC_WDT_VERSION}/pmic_wdt_phy.c
	${CMAKE_CURRENT_SOURCE_DIR}/pmic_wdt_hal.c
)

target_include_directories(boot_driver
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${AIC_PMIC_DIR}>
)
