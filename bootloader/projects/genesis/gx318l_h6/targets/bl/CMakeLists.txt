#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

target_add_scatter_file(${AIC_BL_NAME}
    $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/gx_board.ld>
)

target_include_directories(boot_arch
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/config/mpu>
)
