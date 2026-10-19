#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

include(${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/gx316l_h4/targets/${SUB_TARGET}/defconfig.cmake)
#subsys
set(CONFIG_GX316L_H3 ON  CACHE BOOL "GX316L_H3 BOARD")
add_definitions(-DCONFIG_GX316L_H3)

set(CONFIG_BCD ON CACHE BOOL "Add bcd driver.")
add_definitions(-DCONFIG_BCD)

set(CONFIG_HLZMA_SUPPORT ON CACHE BOOL "Enable hlzma support.")
add_definitions(-DCONFIG_HLZMA_SUPPORT)

set(CONFIG_BCD_DUMMY ON CACHE BOOL "Add bcd dummy driver.")
add_definitions(-DCONFIG_BCD_DUMMY)