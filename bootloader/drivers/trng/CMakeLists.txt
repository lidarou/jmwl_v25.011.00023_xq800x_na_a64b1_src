#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2025 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_sources_ifdef(CONFIG_TRNG boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/trng.c
)

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/>
)

