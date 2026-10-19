# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
            CONFIG_USE_ALV_BTNMATRIX
            ${CMAKE_CURRENT_LIST_DIR}/alv_btnmatrix.c
)

aic_library_sources_ifdef(
            CONFIG_USE_APP_FS
            ${CMAKE_CURRENT_LIST_DIR}/app_fs.c
)

aic_library_sources_ifdef(
            CONFIG_USE_SQUARELINE_COMMON_CODE
            ${CMAKE_CURRENT_LIST_DIR}/ui_helpers.c
            ${CMAKE_CURRENT_LIST_DIR}/ui_comp.c
)

aic_library_sources_ifdef(
            CONFIG_USE_ALV_MENU
            ${CMAKE_CURRENT_LIST_DIR}/alv_menu.c
)

aic_library_sources(
            ${CMAKE_CURRENT_LIST_DIR}/aic_str.c
            ${CMAKE_CURRENT_LIST_DIR}/backlight/aic_srv_backlight.c
            ${CMAKE_CURRENT_LIST_DIR}/backlight/backlight.c
)

aic_library_sources_ifdef(
            CONFIG_APP_SETTINGS
            ${CMAKE_CURRENT_LIST_DIR}/factoryreset/factoryreset.c
)

aic_library_sources_ifdef(
            CONFIG_APP_SETTINGS_FP
            ${CMAKE_CURRENT_LIST_DIR}/factoryreset/factoryreset.c
)

aic_library_sources_ifdef(
            CONFIG_AUDIO_SUPPORT
            ${CMAKE_CURRENT_LIST_DIR}/audio/app_audio.c
            ${CMAKE_CURRENT_LIST_DIR}/audio/app_audio_mp3.c
            ${CMAKE_CURRENT_LIST_DIR}/audio/app_tts.c
)

aic_library_sources_ifdef(
            CONFIG_LV_CONF_MINIMAL
            ${CMAKE_CURRENT_LIST_DIR}/sleep/app_sleep.c
)

aic_library_include_directories_ifdef(
            CONFIG_AUDIO_SUPPORT
            ${CMAKE_CURRENT_LIST_DIR}/audio
)

aic_library_include_directories(
            ${CMAKE_CURRENT_LIST_DIR}/
            ${CMAKE_CURRENT_LIST_DIR}/sleep
            ${CMAKE_CURRENT_LIST_DIR}/backlight
            ${CMAKE_CURRENT_LIST_DIR}/factoryreset
)

