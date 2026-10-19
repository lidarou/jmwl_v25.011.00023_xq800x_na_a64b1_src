# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_sources(
    ftlcore.c
    ftlblock.c
    #ftlrandom.c
)

add_subdirectory_ifdef(CONFIG_AZURE_LEVELX levelx)
