# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

target_sources(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/ftlcore.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ftlblock.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ftlrandom.c
)

add_subdirectory_ifdef(CONFIG_AZURE_LEVELX levelx)
