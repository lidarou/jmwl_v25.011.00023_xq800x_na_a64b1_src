# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_APP_PHOTO ${SOURCES})

file(GLOB SOURCES ./app_photo/*.c)
aic_library_sources_ifdef(CONFIG_APP_PHOTO ${SOURCES})

file(GLOB SOURCES ./res/*.c)
aic_library_sources_ifdef(CONFIG_APP_PHOTO ${SOURCES})

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/app/photo/
)
