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
        ${CMAKE_CURRENT_SOURCE_DIR}/console.c
        ${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_AIC_UART_VERSION}/aic_serial.c
)

target_sources_ifdef(CONFIG_DOWNLOAD_UART boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_AIC_UART_VERSION}/dload_uart.c
)