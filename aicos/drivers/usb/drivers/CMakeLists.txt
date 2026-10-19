# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/usb_driver.c
    ${CMAKE_CURRENT_LIST_DIR}/usb_dma.c
    ${CMAKE_CURRENT_LIST_DIR}/usb_phy.c
)

aic_library_sources_ifdef(CONFIG_USB_DETECT_BCD
    ${CMAKE_CURRENT_LIST_DIR}/usb_bcd.c
)

aic_library_sources_ifdef(CONFIG_USB_DETECT_GPIO
    ${CMAKE_CURRENT_LIST_DIR}/usb_gpio_detect.c
)