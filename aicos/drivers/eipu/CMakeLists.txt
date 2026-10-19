# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_EIPU_DRIVER
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_EIPU_VERSION}/eipu_drv.c
)