# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_sources_ifdef(CONFIG_NET_DRIVER_DIRECT_FORWARD ${CMAKE_CURRENT_LIST_DIR}/src/hashtable.c)
aic_library_sources_ifdef(CONFIG_NET_DRIVER_DIRECT_FORWARD ${CMAKE_CURRENT_LIST_DIR}/src/aic_net_share_port_mgr.c)
