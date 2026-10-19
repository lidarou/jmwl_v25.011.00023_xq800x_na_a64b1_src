# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_tcpip_main.c
)

aic_library_sources_ifdef(CONFIG_NET_AT_PING ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_ping.c)
aic_library_sources_ifdef(CONFIG_NET_AT_SNTP ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_sntp.c)
aic_library_sources_ifdef(CONFIG_NET_AT_TLS ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_tls.c)
aic_library_sources_ifdef(CONFIG_NET_AT_TEST ${CMAKE_CURRENT_LIST_DIR}/test/aic_at_tcpip_test.c)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
)
