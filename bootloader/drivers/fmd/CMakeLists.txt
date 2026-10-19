#/****************************************************************************/
#/*                                                                          */
#/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
#/*                                                                          */
#/****************************************************************************/

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

target_sources(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/fmdcore.c
        ${CMAKE_CURRENT_SOURCE_DIR}/fmdpart.c
)

target_sources_ifdef(CONFIG_FMDPART_UPDATE boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/fmdpart_update.c
)

target_sources_ifdef(CONFIG_FMDSTRESS_TEST boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/fmdstress_test.c
)

add_subdirectory(snor)

add_subdirectory_ifdef(CONFIG_FTL ftl)