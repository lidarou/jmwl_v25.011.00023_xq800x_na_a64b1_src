# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/drivers/sound/ipc
    ${AIC_SOURCE_ROOT}/mm/audio
)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/volte_simu.c
)

aic_library_sources_ifdef(CONFIG_VOLTE_SIMU
    ${SRCS}
)
