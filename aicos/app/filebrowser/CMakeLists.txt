# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
file(GLOB_RECURSE SOURCES ./*.c)

aic_library_sources_ifdef(
    CONFIG_APP_FILEBROWSER
    ${SOURCES}
)

aic_library_include_directories_ifdef(
    CONFIG_APP_FILEBROWSER
    ${AIC_SOURCE_ROOT}/app/filebrowser/
)