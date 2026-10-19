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
        ${CMAKE_CURRENT_SOURCE_DIR}/usb_dma.c
)

target_sources_ifdef(CONFIG_BOOT_USB boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/usb_driver_bl.c
        ${CMAKE_CURRENT_SOURCE_DIR}/class/cdc-acm_bl.c
)

target_sources_ifdef(CONFIG_DOWNLOAD_USB boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/usb_driver.c
        ${CMAKE_CURRENT_SOURCE_DIR}/class/cdc-acm.c
)