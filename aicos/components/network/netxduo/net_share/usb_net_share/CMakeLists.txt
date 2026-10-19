# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_sources_ifdef(CONFIG_USB_NET_SHARE ${CMAKE_CURRENT_LIST_DIR}/src/aic_usb_net_share_mgr.c)
aic_library_sources_ifdef(CONFIG_USB_NET_SHARE ${CMAKE_CURRENT_LIST_DIR}/src/aic_usb_share_at_handle.c)
