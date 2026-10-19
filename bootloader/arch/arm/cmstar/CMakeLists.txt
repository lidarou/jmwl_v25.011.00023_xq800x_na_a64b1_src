#/*****************************************************************************/
#/*                                                                           */
#/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
#/*                                                                           */
#/*****************************************************************************/

target_include_directories(boot_arch
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(boot_arch
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/start.S
        ${CMAKE_CURRENT_SOURCE_DIR}/crt0.s
        $<$<BOOL:${CONFIG_BL_CPU_CACHE}>: ${CMAKE_CURRENT_LIST_DIR}/cache.c>
        $<$<BOOL:${CONFIG_BL_CPU_MPU}>: ${CMAKE_CURRENT_LIST_DIR}/mpu.c>
        ${CMAKE_CURRENT_SOURCE_DIR}/boot_next.c
)

target_sources_ifdef(CONFIG_MBL_BOOT boot_arch
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mbl_boot.c
)

target_sources_ifdef(CONFIG_BOOT boot_arch
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/boot.c
)