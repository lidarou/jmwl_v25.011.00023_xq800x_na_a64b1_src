# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(CONFIG_NET_EXTERNEL_API_TEST ${CMAKE_CURRENT_LIST_DIR}/api_test/aic_net_api_test.c)

aic_library_sources_ifdef(CONFIG_NETXDUO_NAT_TEST ${CMAKE_CURRENT_LIST_DIR}/nat_test/aic_netx_nat_test.c)

aic_library_sources_ifdef(CONFIG_NET_AIC_PACKET_TEST ${CMAKE_CURRENT_LIST_DIR}/packet_test/aic_packet_test.c)
