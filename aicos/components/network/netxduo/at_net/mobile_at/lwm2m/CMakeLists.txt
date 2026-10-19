# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/cm_at_mlwm2m.c
    ${CMAKE_CURRENT_LIST_DIR}/src/cm_at_mlw2m_obj_mgr.c
    ${CMAKE_CURRENT_LIST_DIR}/src/cm_at_split_paras.c
)

aic_library_sources(${SOURCES})
