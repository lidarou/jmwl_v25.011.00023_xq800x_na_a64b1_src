# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/nx_ipsec.c
)

aic_library_sources_ifdef(CONFIG_NET_IPSEC_IPV4_UDP_TEST ${CMAKE_CURRENT_LIST_DIR}/test/aic_ipsec_ipv4_udp_test.c)
aic_library_sources_ifdef(CONFIG_NET_IPSEC_IPV4_TCP_TEST ${CMAKE_CURRENT_LIST_DIR}/test/aic_ipsec_ipv4_tcp_test.c)
aic_library_sources_ifdef(CONFIG_NET_IPSEC_IPV6_UDP_TEST ${CMAKE_CURRENT_LIST_DIR}/test/aic_ipsec_ipv6_udp_test.c)
aic_library_sources_ifdef(CONFIG_NET_IPSEC_IPV6_TCP_TEST ${CMAKE_CURRENT_LIST_DIR}/test/aic_ipsec_ipv6_tcp_test.c)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

