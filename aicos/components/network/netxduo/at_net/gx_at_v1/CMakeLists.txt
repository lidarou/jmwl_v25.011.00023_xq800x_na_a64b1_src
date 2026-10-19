# General configuration options
#
# Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(CONFIG_NET_AT_HTTP http_at)
add_subdirectory_ifdef(CONFIG_NET_AT_TCPIP tcpip_at)
add_subdirectory_ifdef(CONFIG_NET_AT_HTTPS https_at)
add_subdirectory_ifdef(CONFIG_NET_AT_FTP ftp_at)
add_subdirectory_ifdef(CONFIG_NET_AT_MQTT mqtt_at)
add_subdirectory_ifdef(CONFIG_NET_AT_IPERF iperf_at)
