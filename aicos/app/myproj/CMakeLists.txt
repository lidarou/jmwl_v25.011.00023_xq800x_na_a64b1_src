# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc/
    ${CMAKE_CURRENT_LIST_DIR}/src/
)

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/src/my_gpio.c
    ${CMAKE_CURRENT_LIST_DIR}/src/my_uart.c
    ${CMAKE_CURRENT_LIST_DIR}/src/my_log.c
    ${CMAKE_CURRENT_LIST_DIR}/src/my_tool.c
    ${CMAKE_CURRENT_LIST_DIR}/src/my_main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/my_mqtt.c
    ${CMAKE_CURRENT_LIST_DIR}/src/my_gps.c
)


