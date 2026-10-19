# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_sources_ifdef(CONFIG_PPP_DIAL ${CMAKE_CURRENT_LIST_DIR}/src/aic_ppp_dial_mgr.c)
aic_library_sources_ifdef(CONFIG_PPP_DIAL ${CMAKE_CURRENT_LIST_DIR}/src/aic_ppp_dial_at_handle.c)
