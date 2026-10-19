
set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/aic_network_pcap.c
)

aic_library_sources(${SOURCES})

aic_include_directories( ${CMAKE_CURRENT_LIST_DIR})