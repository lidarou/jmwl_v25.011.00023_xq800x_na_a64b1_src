# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_sources_ifdef(CONFIG_IMS_NET_MGR ${CMAKE_CURRENT_LIST_DIR}/src/aic_ims_net_mgr.c)

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
)
