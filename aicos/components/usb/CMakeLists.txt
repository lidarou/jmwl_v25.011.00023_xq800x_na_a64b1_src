# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(aic_usb
    VERSION 1.0.0
    LANGUAGES C
)

#
# Create Library and add Curent Library to project library list
# aic_library_named is register open source code library
#
aic_library_named(${PROJECT_NAME})

#aic_library_sources_ifdef(CONFIG_USB_SERVICE
    # {{BEGIN_TARGET_SOURCES}}
    # ${CMAKE_CURRENT_LIST_DIR}/usb_app_test.c
    # {{END_TARGET_SOURCES}}
#)

aic_library_sources_ifdef(CONFIG_CDC_ACM_INSTANCE
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/cdc_acm.c
    # {{END_TARGET_SOURCES}}
)

aic_library_sources_ifdef(CONFIG_RNDIS_INSTANCE
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/rndis.c
    # {{END_TARGET_SOURCES}}
)

aic_library_sources_ifdef(CONFIG_CDC_ECM_INSTANCE
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/cdc_ecm.c
    # {{END_TARGET_SOURCES}}
)

aic_library_sources_ifdef(CONFIG_USB_STORAGE_INSTANCE
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/storage.c
    # {{END_TARGET_SOURCES}}
)

aic_library_sources_ifdef(CONFIG_USB_SERVICE
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/usb_service.c
    # {{END_TARGET_SOURCES}}
)

add_subdirectory_ifdef(CONFIG_AZURE_USBX    usbx)

aic_include_directories(
    ./inc
)