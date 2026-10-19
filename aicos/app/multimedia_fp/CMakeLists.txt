# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
        ./
        ./app_240x320/record
        ./app_240x320/music
        ./app_240x320/music/id3v2lib/include/
        ./app_240x320/music/id3v2lib/src/
)

file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_APP_MULTIMEDIA_FP ${SOURCES})


if(CONFIG_RESOLUTION_240x320)
    file(GLOB_RECURSE  SOURCES ./app_240x320/*.c)
    aic_library_sources_ifdef(CONFIG_APP_MULTIMEDIA_FP ${SOURCES})
endif()
