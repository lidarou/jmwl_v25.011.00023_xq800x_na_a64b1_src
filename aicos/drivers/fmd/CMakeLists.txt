# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    fmdcore.c
    fmdpart.c
)

add_subdirectory_ifdef(CONFIG_FTL ftl)

add_subdirectory_ifdef(CONFIG_FMD_SNOR snor)
