cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

#############################################################
# Public curent header file to total project
# Add the directory to INTERFACE LIBRARY
#############################################################
#
aic_include_directories(
    ./console/inc
    ./init/inc
    ./assert/inc
)

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/misc/inc
    ${CMAKE_CURRENT_LIST_DIR}/miscdata/inc
)

aic_include_directories_ifdef(CONFIG_CRYPTO_MBEDTLS_3
    ./crypto/mbedtls-3.3.0/include
)

aic_include_directories_ifdef(CONFIG_CRYPTO_MBEDTLS_2
    ./crypto/mbedtls-2.1.8/include
)

aic_include_directories_ifndef(CONFIG_CRYPTO_MBEDTLS
    ./crypto/mbedtls-3.3.0/include
)

aic_include_directories_ifdef(CONFIG_SUBSYS_AP
    ./misc/mdfile/inc
)

aic_include_directories_ifdef(CONFIG_ALOG_MSG_SUPPORT
    ./almsg/inc
    ${AIC_SOURCE_ROOT}/comm/ps/code/inc
)

aic_include_directories_ifdef(CONFIG_ALOG_SUPPORT
    ./alog/inc
    ./alog/pcap
)

aic_include_directories_ifdef(CONFIG_NET_PCAP_DUMP
    ./alog/pcap
)

aic_include_directories_ifdef(CONFIG_AIPC_SUPPORT
    ./aipc/inc
)

aic_include_directories_ifdef(CONFIG_FOTA_SUPPORT
    ./fota/inc
)

aic_include_directories_ifdef(CONFIG_SUBSYS_CTRL_SUPPORT
    ./subsys_ctrl/inc
)

aic_include_directories_ifdef(CONFIG_CONNECT_MGR
    ./connmgr/inc
)

aic_include_directories_ifdef(CONFIG_DIAG_SUPPORT
    ./diag/inc
)

aic_include_directories_ifdef(CONFIG_AT_PROC
    ./include/at
)

aic_include_directories_ifdef(CONFIG_WIFI_SCAN
    ./wifi/wifi_scan/inc
)

aic_include_directories_ifdef(CONFIG_BLT_SUPPORT
    ./blt/inc
)

aic_include_directories_ifdef(CONFIG_SUBSYS_AP
    ${CMAKE_CURRENT_LIST_DIR}/nvram/ap/inc
)
aic_include_directories_ifdef(CONFIG_SUBSYS_CP
    ${CMAKE_CURRENT_LIST_DIR}/nvram/nvfile/inc
    ${CMAKE_CURRENT_LIST_DIR}/nvram/cp/inc
)

aic_include_directories_ifdef(CONFIG_SECURE_BOOT_CORE
    ./include/secure
)
aic_include_directories_ifdef(CONFIG_SUBSYS_AP
    ./portable/include
)

aic_include_directories_ifdef(CONFIG_D2D_L3
    ${CMAKE_CURRENT_LIST_DIR}/d2d/inc
    ${CMAKE_CURRENT_LIST_DIR}/d2d/mdfile/inc
)

aic_include_directories_ifdef(CONFIG_D2D_L3_DUMMY
    ${CMAKE_CURRENT_LIST_DIR}/d2d_l3_dummy/inc
    ${CMAKE_CURRENT_LIST_DIR}/d2d_l3_dummy/mdfile/inc
)

aic_include_directories_ifdef(CONFIG_D2D_APP
    ${CMAKE_CURRENT_LIST_DIR}/d2d_app
)

aic_library_import_ifdef(CONFIG_D2D_CODEC_ISELP
    iselp
    ${CMAKE_CURRENT_LIST_DIR}/d2d_app/iselp/libiselp.a
)

aic_library_import_ifdef(CONFIG_D2D_CODEC_PRIVOICE
    privoice
    ${CMAKE_CURRENT_LIST_DIR}/d2d_app/privoice_codec/libprivoice.a
)

aic_include_directories(./tmr)

#
# Create Library and add Curent Library to project library list
# aic_sdk_library is register close source code library
# Note!!!!!!
# The following code would be close source code
aic_sdk_library(cs_components)

add_sdk_subdirectory_ifdef(CONFIG_ALOG_MSG_SUPPORT      almsg)
add_sdk_subdirectory_ifdef(CONFIG_AIPC_SUPPORT          aipc)
add_sdk_subdirectory_ifdef(CONFIG_FOTA_SUPPORT          fota/src)
add_sdk_subdirectory_ifdef(CONFIG_NVRAM_SUPPORT         nvram)
add_sdk_subdirectory_ifdef(CONFIG_MCX_SUPPORT           mcx)
add_sdk_subdirectory_ifdef(CONFIG_MISC_SUPPORT          misc)
add_sdk_subdirectory_ifdef(CONFIG_SECURE_BOOT_CORE      secure)
add_sdk_subdirectory_ifdef(CONFIG_WIFI_SCAN             wifi)
add_sdk_subdirectory_ifdef(CONFIG_D2D_L3_DUMMY          d2d_l3_dummy)
add_sdk_subdirectory_ifdef(CONFIG_D2D_APP               d2d_app)
add_sdk_subdirectory(bus)
add_sdk_subdirectory(portable)

#
# Create Library and add Curent Library to project library list
# aic_library_named is register open source code library
# Note!!!!!!
# The following code would be open source code
aic_library_named(aic_components)

add_subdirectory(assert)
add_subdirectory(at)
add_subdirectory(console)
add_subdirectory(init)
add_subdirectory(module)
add_subdirectory(network)
add_subdirectory(prodinfo)
add_subdirectory(tele)
add_subdirectory(usb)
add_subdirectory(tmr)

add_subdirectory_ifdef(CONFIG_TELE_SERVICE_EX       tele/service_ex)
add_subdirectory_ifdef(CONFIG_ALOG_SUPPORT          alog)
add_subdirectory_ifdef(CONFIG_LOCATION              location)
add_subdirectory_ifdef(CONFIG_BENCHMARK             benchmark)
add_subdirectory_ifdef(CONFIG_CONNECT_MGR           connmgr)
add_subdirectory_ifdef(CONFIG_CRYPTO_MBEDTLS        crypto)
add_subdirectory_ifdef(CONFIG_DIAG_SUPPORT          diag)
add_subdirectory_ifdef(CONFIG_BLT_SUPPORT           blt)
add_subdirectory_ifdef(CONFIG_GRAPHICS              graphics)
add_subdirectory_ifdef(CONFIG_GPS                   gps)
add_subdirectory_ifdef(CONFIG_FS                    fs)
add_subdirectory_ifdef(CONFIG_SUBSYS_CTRL_SUPPORT   subsys_ctrl)
add_subdirectory_ifdef(CONFIG_AIC_CTRL_SUPPORT      aic_ctrl)
add_subdirectory_ifdef(CONFIG_NVRAM_SUPPORT         nvram/cfg)
add_subdirectory_ifdef(CONFIG_MISC_SUPPORT          misc/cfg)
add_subdirectory_ifdef(CONFIG_USE_ICONV             iconv)
add_subdirectory_ifdef(CONFIG_FOTA_SUPPORT          fota/port)
add_subdirectory_ifdef(CONFIG_D2D_L3                d2d)
add_subdirectory_ifdef(CONFIG_D2D                   d2d_mgr)
