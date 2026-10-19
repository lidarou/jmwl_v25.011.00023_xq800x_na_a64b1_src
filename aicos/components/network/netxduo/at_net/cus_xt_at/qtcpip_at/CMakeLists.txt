# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_qtcpip.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_at_qssl.c
)

aic_library_sources_ifdef(CONFIG_NET_AT_QTLS_TEST ${CMAKE_CURRENT_LIST_DIR}/test/aic_at_qssl_test.c)

aic_library_sources(${SOURCES})
