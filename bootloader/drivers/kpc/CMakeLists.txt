#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_include_directories_ifdef(CONFIG_KPC boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_sources_ifdef(CONFIG_KPC boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/kpc.c
)
