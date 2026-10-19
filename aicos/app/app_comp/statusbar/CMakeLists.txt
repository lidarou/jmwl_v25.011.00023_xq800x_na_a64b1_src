# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
aic_library_include_directories(
        ./
        ./flow/)
file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_COMP_STATUSBAR ${SOURCES})

if(CONFIG_RESOLUTION_128x128)
    aic_library_include_directories(./ui_128x128/)
    file(GLOB_RECURSE SOURCES ./ui_128x128/*.c)
else()
    aic_library_include_directories(./ui/)
    file(GLOB_RECURSE SOURCES ./ui/*.c)
endif()

aic_library_sources_ifdef(CONFIG_COMP_STATUSBAR ${SOURCES})
