#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_sources(boot_driver
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/wdt.c
)

target_include_directories(boot_driver
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${AIC_PMIC_DIR}>
)
