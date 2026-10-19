#/****************************************************************************/
#/*                                                                          */
#/*     Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.              */
#/*                                                                          */
#/****************************************************************************/

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_sources(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/core.c
        ${CMAKE_CURRENT_LIST_DIR}/aic_checksum.c
        ${CMAKE_CURRENT_LIST_DIR}/dl_chn_reset.c
)

target_sources_ifdef(CONFIG_CMDLINE boot_driver
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/cmdline.c
)