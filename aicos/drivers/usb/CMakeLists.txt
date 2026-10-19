# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
add_subdirectory_ifdef(CONFIG_AZURE_USBX    usbx)
add_subdirectory_ifdef(CONFIG_USB_AIC    usbx_aic)
add_subdirectory_ifdef(CONFIG_USB_AIC    drivers)
aic_include_directories(
    ./inc
)