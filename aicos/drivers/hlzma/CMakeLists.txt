cmake_minimum_required(VERSION 3.13)

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/hlzma_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_HLZMA_VER}/hlzma_phy.c
)
