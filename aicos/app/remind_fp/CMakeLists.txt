# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
        ./
)

file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_APP_REMIND_FP ${SOURCES})


if(CONFIG_RESOLUTION_240x320)
    file(GLOB_RECURSE  SOURCES ./app_240x320/*.c)
    aic_library_sources_ifdef(CONFIG_APP_REMIND_FP ${SOURCES})
endif()
