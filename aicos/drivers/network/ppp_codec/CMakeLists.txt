
set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_ppp_codec_core.c
)

add_subdirectory_ifdef(CONFIG_NET_PPP_CODEC_PHY_V1 src/phy_v1)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
)
