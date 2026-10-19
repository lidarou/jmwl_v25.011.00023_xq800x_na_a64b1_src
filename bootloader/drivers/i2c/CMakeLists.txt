#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_sources(boot_driver
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/i2c_phy.c
    ${CMAKE_CURRENT_LIST_DIR}/i2c_core.c
)

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)