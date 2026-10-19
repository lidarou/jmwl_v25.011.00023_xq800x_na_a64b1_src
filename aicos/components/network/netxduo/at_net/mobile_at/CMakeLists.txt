# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/common/src/cm_at_common.c
)

aic_library_sources(${SOURCES})

if(CONFIG_CM_MQTT_AT OR CONFIG_GX_MQTT_AT)
    add_subdirectory(mqtt)
endif()

if(CONFIG_CM_HTTP_AT OR CONFIG_GX_HTTP_AT)
    add_subdirectory(http)
endif()

if(CONFIG_CUSTOMER_CM_AT OR CONFIG_GX_AT_V2)
    add_subdirectory(tcpip)
endif()

add_subdirectory_ifdef(CONFIG_CM_LWM2M_AT lwm2m)
