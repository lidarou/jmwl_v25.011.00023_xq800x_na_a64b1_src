# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_include_directories(inc)

aic_include_directories(
    ${AIC_SOURCE_ROOT}/drivers/fmd/
)

add_subdirectory(src)
