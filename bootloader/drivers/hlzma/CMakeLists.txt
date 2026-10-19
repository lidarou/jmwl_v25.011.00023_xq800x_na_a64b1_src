#/****************************************************************************/
#/*                                                                          */
#/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
#/*                                                                          */
#/****************************************************************************/

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

target_sources(boot_driver
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hlzma_hal.c
)

add_subdirectory(r1p0)
