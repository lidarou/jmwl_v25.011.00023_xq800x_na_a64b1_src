# General configuration options
#
# Copyright 2023 by AIC Micro Corporation. All rights reserved.

aic_library_named(ssv_wifi)

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/host
    ${CMAKE_CURRENT_LIST_DIR}/host/app
    ${CMAKE_CURRENT_LIST_DIR}/host/app/cli
    ${CMAKE_CURRENT_LIST_DIR}/host/app/cli/cmds
    ${CMAKE_CURRENT_LIST_DIR}/host/app/netapp
    ${CMAKE_CURRENT_LIST_DIR}/host/app/netmgr
    ${CMAKE_CURRENT_LIST_DIR}/host/drv
    ${CMAKE_CURRENT_LIST_DIR}/host/drv/sdio
    ${CMAKE_CURRENT_LIST_DIR}/host/hal
    ${CMAKE_CURRENT_LIST_DIR}/host/hal/ssv6020
	${CMAKE_CURRENT_LIST_DIR}/host/hal/ssv6020/firmware/ssv6020C
	${CMAKE_CURRENT_LIST_DIR}/host/include
	${CMAKE_CURRENT_LIST_DIR}/host/include/priv
	${CMAKE_CURRENT_LIST_DIR}/host/include/priv/hw
	${CMAKE_CURRENT_LIST_DIR}/host/netstack_wrapper/aic_tcpip
	${CMAKE_CURRENT_LIST_DIR}/host/os_wrapper/aic_os
	${CMAKE_CURRENT_LIST_DIR}/host/platform/aic
)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.c)

aic_library_sources(${SOURCES})

aic_library_import(wifi_lib_core ${AIC_SOURCE_ROOT}/vendors/ssv_wifi/host/lib/wifi_lib_core.a)
