# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

file(GLOB SOURCES src/*.c)

aic_library_include_directories(
    ./src
    ./include
)
aic_library_sources(${SOURCES})
