# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_lwm2m_client.c
)

# aic_library_sources_ifdef(CONFIG_NET_LWM2M ${CMAKE_CURRENT_LIST_DIR}/src/aic_lwm2m_client_test.c)

aic_library_sources(${SOURCES})

