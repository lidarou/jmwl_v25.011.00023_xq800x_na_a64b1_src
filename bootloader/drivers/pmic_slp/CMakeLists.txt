#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_sources_ifdef(CONFIG_PMIC_SLP boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/pmic_slp.c
)
