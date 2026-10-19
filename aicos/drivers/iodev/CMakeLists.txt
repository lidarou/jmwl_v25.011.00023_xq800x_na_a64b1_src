# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/aic_io_dev.c
    ${CMAKE_CURRENT_LIST_DIR}/drv_file.c
)


aic_library_sources_ifndef(CONFIG_DRIVER_UART_AIC
    ${CMAKE_CURRENT_LIST_DIR}/drv_usart.c
)