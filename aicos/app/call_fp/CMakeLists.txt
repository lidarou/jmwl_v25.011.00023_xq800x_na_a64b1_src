# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
        ./
        ./flow
)

if(CONFIG_RESOLUTION_240x320)
    file(GLOB SOURCES ./*.c)
    aic_library_sources_ifdef(CONFIG_APP_CALL_FP ${SOURCES})
    file(GLOB_RECURSE SOURCES ./flow/*.c)
    aic_library_sources_ifdef(CONFIG_APP_CALL_FP ${SOURCES})
    file(GLOB_RECURSE  SOURCES ./app_240x320/*.c)
    aic_library_sources_ifdef(CONFIG_APP_CALL_FP ${SOURCES})
endif()