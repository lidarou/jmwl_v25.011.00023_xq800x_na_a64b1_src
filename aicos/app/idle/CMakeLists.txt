# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_LIST_DIR}/flow
)
if(CONFIG_RESOLUTION_128x128)
    file(GLOB_RECURSE  SOURCES ./ui_128x128/app_idle/*.c)
    # delete squareline project
    file(GLOB_RECURSE REMOVE_SOURCES ./ui_128x128/app_idle/squareline_prj/*)
    list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
else()
    file(GLOB_RECURSE  SOURCES ./ui/app_idle/*.c)
    file(GLOB_RECURSE REMOVE_SOURCES ./ui/app_idle/squareline_prj/*)
    list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
endif()

aic_library_sources_ifdef(CONFIG_APP_IDLE ${SOURCES})

