# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/flow
)

if(CONFIG_RESOLUTION_128x128)
    aic_library_include_directories(./ui_128x128/)
    file(GLOB_RECURSE SOURCES ./ui_128x128/*.c)
    aic_library_sources_ifdef(CONFIG_APP_SETTINGS ${SOURCES})
else()
    aic_library_include_directories(./ui/)
    aic_library_include_directories(./ui/win_dual_sim_select)
    file(GLOB_RECURSE SOURCES ./ui/*.c)
    aic_library_sources_ifdef(CONFIG_APP_SETTINGS ${SOURCES})
endif()

