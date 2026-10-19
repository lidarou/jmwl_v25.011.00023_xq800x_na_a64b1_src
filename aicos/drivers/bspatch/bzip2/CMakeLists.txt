# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/blocksort.c
    ${CMAKE_CURRENT_LIST_DIR}/bzlib.c
    ${CMAKE_CURRENT_LIST_DIR}/compress.c
    ${CMAKE_CURRENT_LIST_DIR}/crctable.c
    ${CMAKE_CURRENT_LIST_DIR}/decompress.c
    ${CMAKE_CURRENT_LIST_DIR}/huffman.c
    ${CMAKE_CURRENT_LIST_DIR}/randtable.c
)

aic_library_sources(${SRCS})
