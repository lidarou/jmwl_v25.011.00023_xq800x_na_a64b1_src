# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/drivers/sound/driver
    ${AIC_SOURCE_ROOT}/drivers/sound/ipc
)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/adsp_mng.c
)

aic_library_sources_ifdef(CONFIG_ADSP_MNG
    ${SRCS}
)
