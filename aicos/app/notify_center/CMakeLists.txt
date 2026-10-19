# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

    file(GLOB SOURCES ./*.c)
    aic_library_sources_ifdef(CONFIG_APP_NOTIFY_CENTER ${SOURCES})

if(CONFIG_RESOLUTION_128x128)
    file(GLOB_RECURSE SOURCES ./app_notify_center_128x128/*.c)
    aic_library_sources_ifdef(CONFIG_APP_NOTIFY_CENTER ${SOURCES})
endif()
