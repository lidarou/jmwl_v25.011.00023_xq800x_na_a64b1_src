# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

target_sources(boot_driver
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/blocksort.c
        ${CMAKE_CURRENT_SOURCE_DIR}/bzlib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/compress.c
        ${CMAKE_CURRENT_SOURCE_DIR}/crctable.c
        ${CMAKE_CURRENT_SOURCE_DIR}/decompress.c
        ${CMAKE_CURRENT_SOURCE_DIR}/huffman.c
        ${CMAKE_CURRENT_SOURCE_DIR}/randtable.c
)
