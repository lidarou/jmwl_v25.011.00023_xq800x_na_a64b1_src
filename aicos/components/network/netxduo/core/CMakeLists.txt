cmake_minimum_required(VERSION 3.13)

string(REPLACE "-" "_" THREADX_ARCH ${CONFIG_CPU_NAME})
if ("${COMPILER}" STREQUAL "gcc")
    set(THREADX_TOOLCHAIN "gnu")
else()
    set(THREADX_TOOLCHAIN ${COMPILER})
endif()

if ("${THREADX_ARCH}" STREQUAL "")
    message(FATAL_ERROR "Error: THREADX_ARCH not defined")
endif()
if ("${THREADX_TOOLCHAIN}" STREQUAL "")
    message(FATAL_ERROR "Error: THREADX_TOOLCHAIN not defined")
endif()

# aic_sdk_library is register close source code library
aic_sdk_library(netxduo)

# Include NETXDUO configurations for compiler
aic_compile_options("SHELL: $<TARGET_PROPERTY:compiler,imacros> ${AIC_CFG_PATH}/config/nx_config.h")

if(NOT CONFIG_SDK_ENABLE)

add_subdirectory_ifdef(CONFIG_NET_COMMON common)
add_subdirectory_ifdef(CONFIG_NET_SECURE nx_secure)
add_subdirectory_ifdef(CONFIG_NET_CRYPTO_LIBRARIES crypto_libraries)
add_subdirectory_ifdef(CONFIG_NET_CRYPTO_LITE_LIBRARIES crypto_lite_libraries)
add_subdirectory_ifdef(CONFIG_NET_ADDONS addons)
add_subdirectory_ifdef(CONFIG_NET_UTILITY utility)

aic_include_directories(${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN}/inc)

else()

# Add the Common/inc directory to the project include list
aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/common/inc
    ${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN}/inc
    ${CMAKE_CURRENT_LIST_DIR}/addons/BSD
    ${CMAKE_CURRENT_LIST_DIR}/addons/dhcp
    ${CMAKE_CURRENT_LIST_DIR}/addons/dns
    ${CMAKE_CURRENT_LIST_DIR}/addons/mqtt
    ${CMAKE_CURRENT_LIST_DIR}/addons/nat
    ${CMAKE_CURRENT_LIST_DIR}/addons/ppp
    ${CMAKE_CURRENT_LIST_DIR}/addons/pppoe
    ${CMAKE_CURRENT_LIST_DIR}/addons/sntp
    ${CMAKE_CURRENT_LIST_DIR}/addons/web
    ${CMAKE_CURRENT_LIST_DIR}/crypto_lite_libraries/inc
)

endif()