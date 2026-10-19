# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/flow
    ${CMAKE_CURRENT_LIST_DIR}/ui_240x320
)

file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})

if(CONFIG_RESOLUTION_240x320)
    file(GLOB SOURCES ./*.c)
    aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})
    file(GLOB SOURCES ./flow/*.c)
    aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})
    file(GLOB SOURCES ./ui_240x320/*.c)
    aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})
    file(GLOB_RECURSE SOURCES ./ui_240x320/win_toolsalarm/*.c)
    aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})
    file(GLOB_RECURSE SOURCES ./ui_240x320/win_toolscalculator/*.c)
    aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})
    file(GLOB_RECURSE SOURCES ./ui_240x320/win_toolscalendar/*.c)
    aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})
    file(GLOB_RECURSE SOURCES ./ui_240x320/win_toolsmenu/*.c)
    aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})
if(CONFIG_AUDIO_TTS)
    file(GLOB_RECURSE SOURCES ./ui_240x320/win_tooltts/*.c)
    aic_library_sources_ifdef(CONFIG_APP_TOOLS_FP ${SOURCES})
endif()
endif()
