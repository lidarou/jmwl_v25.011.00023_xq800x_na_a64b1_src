#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_sources(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/dload_engine.c
        ${CMAKE_CURRENT_SOURCE_DIR}/dload_kit.c
        ${CMAKE_CURRENT_SOURCE_DIR}/dload_packet.c
)

target_sources_ifdef(CONFIG_DL_DOWNLOAD boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/download.c
        ${CMAKE_CURRENT_SOURCE_DIR}/dload_cmd.c
)

target_sources_ifdef(CONFIG_MDL_DOWNLOAD boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mdl/download.c
        ${CMAKE_CURRENT_SOURCE_DIR}/mdl/dload_cmd.c
)