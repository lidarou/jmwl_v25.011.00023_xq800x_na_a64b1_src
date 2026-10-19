# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(CONFIG_USE_NETXDUO netxduo)
add_subdirectory_ifdef(CONFIG_USE_LWIP lwip)
add_subdirectory_ifdef(CONFIG_NET_EXT net_extension)