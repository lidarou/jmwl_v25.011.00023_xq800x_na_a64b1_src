# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/drivers/sound/driver
)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/snd_ipc.c
    ${CMAKE_CURRENT_LIST_DIR}/snd_ipc_async.c
)

aic_library_sources_ifdef(CONFIG_SND_IPC
    ${SRCS}
)
