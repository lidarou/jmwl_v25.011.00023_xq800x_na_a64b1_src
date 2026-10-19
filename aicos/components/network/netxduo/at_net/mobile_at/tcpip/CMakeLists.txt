# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

# set(SOURCES
#     ${CMAKE_CURRENT_LIST_DIR}/src/cm_at_mtcpip.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/cm_at_mssl.c
# )

# aic_library_sources(${SOURCES})

if(CONFIG_CM_TCPIP_AT OR CONFIG_GX_TCPIP_AT)
    aic_library_sources(${CMAKE_CURRENT_LIST_DIR}/src/cm_at_mtcpip.c)
endif()

if(CONFIG_CM_SSL_AT OR CONFIG_GX_SSL_AT)
    aic_library_sources(${CMAKE_CURRENT_LIST_DIR}/src/cm_at_mssl.c)
endif()