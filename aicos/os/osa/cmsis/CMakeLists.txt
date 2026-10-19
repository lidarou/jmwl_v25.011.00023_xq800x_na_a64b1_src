# General configuration options
#
# Copyright 2023 by AIC Micro Corporation. All rights reserved.

aic_library_named(tx_cmsis)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/os/osa/cmsis/include
)

set(TX_CMSIS_SRCS
        ./src/tx_cmsis_os2.c
)

#
# Add source code to library
#
aic_library_sources(${TX_CMSIS_SRCS})
#
# Add test code
#
#add_subdirectory_ifdef(CONFIG_CMSIS_TESTS      tests)
#add_subdirectory(tests)