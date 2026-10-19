#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

include(${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/gx316l_h4/targets/${SUB_TARGET}/defconfig.cmake)
#subsys
set(CONFIG_GX316H_H1 ON  CACHE BOOL "GX316H_H1 BOARD")
add_definitions(-DCONFIG_GX316H_H1)

