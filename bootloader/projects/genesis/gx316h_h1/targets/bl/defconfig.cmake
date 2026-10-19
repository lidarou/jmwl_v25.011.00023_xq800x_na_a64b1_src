#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

include(${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/gx316l_h4/targets/${SUB_TARGET}/defconfig.cmake)
#subsys
set(CONFIG_GX316H_H1 ON  CACHE BOOL "GX316H_H1 BOARD")
add_definitions(-DCONFIG_GX316H_H1)

set(CONFIG_PMIC_CLK ON CACHE BOOL "Add PMIC CLK driver.")
add_definitions(-DCONFIG_PMIC_CLK)

unset(CONFIG_BCD_DUMMY)
remove_definitions(-DCONFIG_BCD_DUMMY)

# support aic fota
#unset(CONFIG_FOTA_SUPPORT CACHE)
#remove_definitions(-DCONFIG_FOTA_SUPPORT)

# Support ailabi fota
# set(CONFIG_ABUP_FOTA ON CACHE BOOL "Add abup fota config")
# add_definitions(-DCONFIG_ABUP_FOTA)
set(CONFIG_COMPRESSED_BIN ON CACHE BOOL "Add compress bin feature")
add_definitions(-DCONFIG_COMPRESSED_BIN)

# Support multi hardware adapt
#set(CONFIG_MHA ON CACHE BOOL "Add multiple hardware adapt config")
#add_definitions(-DCONFIG_MHA)

# Support display api
#set(CONFIG_GRAPHICS ON CACHE BOOL "add display driver.")
#set(CONFIG_PANEL_NAME "st7735" CACHE STRING "lcd panel name")
#add_definitions(-DCONFIG_GRAPHICS)
#set(CONFIG_BACKLIGHT_RGBW ON CACHE BOOL "backlight rgbw driver.")
#set(CONFIG_BACKLIGHT_KPLED ON CACHE BOOL "backlight kpled driver.")