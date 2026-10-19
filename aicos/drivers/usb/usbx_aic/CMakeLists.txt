# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_callback.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_endpoint_create.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_endpoint_reset.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_endpoint_stall.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_endpoint_status.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_frame_number_get.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_function.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_endpoint_destroy.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_frame_number_get.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_initialize.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_initialize_complete.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_transfer_request.c
    ${CMAKE_CURRENT_LIST_DIR}/ux_dcd_transfer_abort.c
    # {{END_TARGET_SOURCES}}
)