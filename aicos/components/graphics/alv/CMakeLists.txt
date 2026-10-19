# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/components/graphics/alv
)

aic_library_sources_ifdef(
        CONFIG_ALV_APP_MGR
        alv_app_mgr.c
)
