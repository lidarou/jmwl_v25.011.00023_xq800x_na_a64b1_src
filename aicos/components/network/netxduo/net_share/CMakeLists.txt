# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_library_sources_ifdef(CONFIG_NET_SHARE_COMMON ${CMAKE_CURRENT_LIST_DIR}/net_share_common/src/aic_net_share_common.c)
aic_library_sources_ifdef(CONFIG_USB_NET_SHARE ${CMAKE_CURRENT_LIST_DIR}/usb_net_share/src/aic_usb_net_share_mgr.c)
aic_library_sources_ifdef(CONFIG_USB_NET_SHARE ${CMAKE_CURRENT_LIST_DIR}/usb_net_share/src/aic_usb_share_at_handle.c)
aic_library_sources_ifdef(CONFIG_PPP_DIAL ${CMAKE_CURRENT_LIST_DIR}/ppp_dial/src/aic_ppp_dial_mgr.c)
aic_library_sources_ifdef(CONFIG_PPP_DIAL ${CMAKE_CURRENT_LIST_DIR}/ppp_dial/src/aic_ppp_dial_at_handle.c)
aic_library_sources_ifdef(CONFIG_NET_DRIVER_DIRECT_FORWARD ${CMAKE_CURRENT_LIST_DIR}/port_mgr/src/aic_net_share_port_mgr.c)
aic_library_sources_ifdef(CONFIG_NET_DRIVER_DIRECT_FORWARD ${CMAKE_CURRENT_LIST_DIR}/port_mgr/src/hashtable.c)
aic_library_sources_ifdef(CONFIG_SOFTAP_SHARE ${CMAKE_CURRENT_LIST_DIR}/softap_mgr/src/aic_softap_mgr.c)
