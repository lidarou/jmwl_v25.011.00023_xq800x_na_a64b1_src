# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_tls_api.c
)

aic_library_sources_ifdef(CONFIG_NET_TLS_API_TEST ${CMAKE_CURRENT_LIST_DIR}/src/aic_tls_api_test.c)

aic_library_sources(${SOURCES})

