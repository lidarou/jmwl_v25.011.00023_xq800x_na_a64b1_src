#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

include(${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/gx316l_h4/targets/${SUB_TARGET}/defconfig.cmake)
#subsys
set(CONFIG_GX316L_H3 ON  CACHE BOOL "GX316L_H3 BOARD")
add_definitions(-DCONFIG_GX316L_H3)

set(CONFIG_PMIC_CLK ON CACHE BOOL "Add PMIC CLK driver.")
add_definitions(-DCONFIG_PMIC_CLK)

set(CONFIG_BCD ON CACHE BOOL "Add bcd driver.")
add_definitions(-DCONFIG_BCD)

set(CONFIG_COMPRESSED_BIN ON CACHE BOOL "Add compress bin feature")
add_definitions(-DCONFIG_COMPRESSED_BIN)

set(CONFIG_BCD_DUMMY ON CACHE BOOL "Add bcd dummy driver.")
add_definitions(-DCONFIG_BCD_DUMMY)