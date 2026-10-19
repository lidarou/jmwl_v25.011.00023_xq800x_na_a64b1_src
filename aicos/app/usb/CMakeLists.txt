# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ./
    ./flow
)

if(CONFIG_RESOLUTION_128x128)
    #file(GLOB_RECURSE SOURCES ./win_usb_128x128/*.c)
else()
    file(GLOB_RECURSE SOURCES ./ui/*.c)
    aic_library_sources_ifdef(CONFIG_APP_USB ${SOURCES})
endif()


