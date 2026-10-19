# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
file(GLOB_RECURSE SOURCES ./*.c)

aic_library_sources_ifdef(
                CONFIG_APP_ENTRY_LVGL_DEMO
                ${SOURCES}
)