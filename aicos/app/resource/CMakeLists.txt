# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

if(CONFIG_RESOLUTION_128x128)
    file(GLOB SOURCES ./image_128x128/*.c)
    aic_library_sources(${SOURCES})
else()
    file(GLOB SOURCES ./image/*.c)
    aic_library_sources(${SOURCES})
endif()

