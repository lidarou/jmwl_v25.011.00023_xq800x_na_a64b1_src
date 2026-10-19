# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_http_comm.c
)

aic_library_sources_ifdef(CONFIG_NET_HTTPS ${CMAKE_CURRENT_LIST_DIR}/src/aic_https_client.c)
#aic_library_sources_ifdef(CONFIG_NET_HTTPS ${CMAKE_CURRENT_LIST_DIR}/src/aic_https_server.c)
aic_library_sources_ifdef(CONFIG_NET_HTTP ${CMAKE_CURRENT_LIST_DIR}/src/aic_http_client.c)
aic_library_sources_ifdef(CONFIG_NET_HTTP_DEMO ${CMAKE_CURRENT_LIST_DIR}/src/aic_http_client_demo.c)

aic_library_sources(${SOURCES})
