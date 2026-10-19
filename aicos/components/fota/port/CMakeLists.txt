# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_ATFOTA
    ${CMAKE_CURRENT_LIST_DIR}/fota_at.c
)

aic_library_sources_ifdef(
    CONFIG_FOTA_HTTPS_SUPPORT
    ${CMAKE_CURRENT_LIST_DIR}/fota_conn_https.c
)

aic_library_sources_ifdef(
    CONFIG_FOTA_HTTP_SUPPORT
    ${CMAKE_CURRENT_LIST_DIR}/fota_conn_http.c
)

aic_library_sources_ifdef(
    CONFIG_FOTA_FTP_SUPPORT
    ${CMAKE_CURRENT_LIST_DIR}/fota_conn_ftp.c
)

aic_library_sources_ifdef(
    CONFIG_FOTA_BY_MCU
    ${CMAKE_CURRENT_LIST_DIR}/fota_ram.c
)

set(SRCS
    ./fota_init.c
    ./fota_at_cmiot.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
