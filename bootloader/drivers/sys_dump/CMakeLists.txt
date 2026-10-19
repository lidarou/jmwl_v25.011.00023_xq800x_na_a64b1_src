#/****************************************************************************/
#/*                                                                          */
#/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
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
        ${CMAKE_CURRENT_SOURCE_DIR}/sys_dump_diag.c
)