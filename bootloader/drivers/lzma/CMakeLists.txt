#/****************************************************************************/
#/*                                                                          */
#/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
#/*                                                                          */
#/****************************************************************************/

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

target_sources(boot_driver
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/LzFind.c
    ${CMAKE_CURRENT_SOURCE_DIR}/LzmaEnc.c
)
