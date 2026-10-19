# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/components/connmgr/inc
)

add_subdirectory_ifdef(CONFIG_DRIVER_UART_XIL xil)

aic_library_sources_ifdef(CONFIG_DRIVER_UART_AIC
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_UART_AIC_VERSION}/serial_phy.c
    # {{END_TARGET_SOURCES}}
)

aic_library_sources_ifdef(CONFIG_DRIVER_UART_AIC
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/serial_hal.c
    # {{END_TARGET_SOURCES}}
)

aic_library_sources_ifdef(CONFIG_PIN_WAKEUP
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/pin_wakeup.c
    # {{END_TARGET_SOURCES}}
)

aic_library_sources_ifdef(CONFIG_DRIVER_GPIO_WAKEUP
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/gpio_wakeup.c
    # {{END_TARGET_SOURCES}}
)

aic_library_sources_ifdef(CONFIG_DRIVER_QUAD_PIN_WAKEUP
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/quad_pin_wakeup.c
    # {{END_TARGET_SOURCES}}
)

aic_include_directories(
    ./inc
)