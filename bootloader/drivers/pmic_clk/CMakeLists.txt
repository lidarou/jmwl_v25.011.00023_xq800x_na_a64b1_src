#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_sources_ifdef(CONFIG_PMIC_CLK boot_driver_cs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/pmic_clk.c
)
