# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(CONFIG_AT_PROC ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_cmd_handle.c)
aic_library_sources_ifdef(CONFIG_AT_PROC ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_interact_cp.c)
aic_library_sources_ifdef(CONFIG_AT_PROC ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_parse_utils.c)
aic_library_sources_ifdef(CONFIG_AT_PROC ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_resp_handle.c)
aic_library_sources_ifdef(CONFIG_AT_PROC ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_urc_handle.c)
aic_library_sources_ifdef(CONFIG_AT_PROC ${CMAKE_CURRENT_LIST_DIR}/src/aic_general_at_handle.c)
aic_library_sources_ifdef(CONFIG_AT_PROC ${CMAKE_CURRENT_LIST_DIR}/src/aic_net_at_resp_handle.c)
aic_library_sources_ifdef(CONFIG_AT_PROC ${CMAKE_CURRENT_LIST_DIR}/src/aic_serial_recv_send.c)
