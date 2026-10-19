# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_named(aic_lwip)
#aic_append_export_library(tcpip)

add_subdirectory_ifdef(CONFIG_TCPIP_LWIP lwip-2.1.3)