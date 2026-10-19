# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(CONFIG_NET_EXTERNEL_API ${CMAKE_CURRENT_LIST_DIR}/src/aic_net_api.c)
aic_library_sources_ifdef(CONFIG_NET_EXTERNEL_API ${CMAKE_CURRENT_LIST_DIR}/src/aic_mbed_dummy_api.c)
aic_library_sources_ifdef(CONFIG_NET_NEW_PACKET_MANAGE ${CMAKE_CURRENT_LIST_DIR}/src/aic_packet_build.c)
aic_library_sources_ifdef(CONFIG_NET_NEW_PACKET_MANAGE ${CMAKE_CURRENT_LIST_DIR}/src/aic_packet_management.c)
aic_library_sources_ifdef(CONFIG_NET_LOW_POWER ${CMAKE_CURRENT_LIST_DIR}/src/aic_net_low_power.c)
aic_library_sources_ifdef(CONFIG_NET_EXTERNEL_API ${CMAKE_CURRENT_LIST_DIR}/src/ip4_addr.c)
aic_library_sources_ifdef(CONFIG_NET_EXTERNEL_API ${CMAKE_CURRENT_LIST_DIR}/src/ip6_addr.c)

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
)
