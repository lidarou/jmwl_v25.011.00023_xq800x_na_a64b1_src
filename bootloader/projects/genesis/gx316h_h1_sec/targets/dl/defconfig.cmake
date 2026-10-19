#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

include(${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/gx316h_h1/targets/${SUB_TARGET}/defconfig.cmake)

set(CONFIG_GX316H_H1_SEC ON  CACHE BOOL "GX316H_H1 BOARD")
add_definitions(-DCONFIG_GX316H_H1_SEC)

set(CONFIG_SECURE_BOOT ON CACHE BOOL "Add SECURE_BOOT driver.")
add_definitions(-DCONFIG_SECURE_BOOT)

set(CONFIG_CRYPTO_SW_ACCELERATOR ON CACHE BOOL "Whether to enable the crypto software accelerator on supported platforms")
add_definitions(-DCONFIG_CRYPTO_SW_ACCELERATOR)
