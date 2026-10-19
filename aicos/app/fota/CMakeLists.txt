# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_APP_FOTA
    app_fota.c
    app_fota_service.c
    app_fota_flow.c
    ./app_fota/ui.c
    ./app_fota/ui_events.c
)

aic_library_include_directories_ifdef(
    CONFIG_APP_FOTA
    ${AIC_SOURCE_ROOT}/app/fota/
)