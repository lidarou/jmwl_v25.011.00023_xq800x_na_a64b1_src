# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_KPLED_VER}/kpled.c
)

aic_library_sources_ifdef(CONFIG_DRIVER_KPLED_DEMO kpled_user_demo.c)