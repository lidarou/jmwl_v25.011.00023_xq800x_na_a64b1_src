# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(CONFIG_CUSTOMER_AT cus_xt_at/qtcpip_at)
add_subdirectory_ifdef(CONFIG_CUSTOMER_AT cus_xt_at/ctcpip_at)

if(CONFIG_CUSTOMER_CM_AT OR CONFIG_GX_AT_V2)
    add_subdirectory(mobile_at)
endif()

add_subdirectory_ifdef(CONFIG_GX_AT_V1 gx_at_v1)
add_subdirectory(tcpip_at_common)
