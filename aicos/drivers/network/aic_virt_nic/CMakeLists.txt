
set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/aic_virt_nic.c
)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/inc
)
