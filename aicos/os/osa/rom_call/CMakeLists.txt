# General configuration options
#
# Copyright 2023 by AIC Micro Corporation. All rights reserved.

set(SRCS
    ./aic_rom_call.c
    ./romcode_cp_dummy.c
    )

#
# Add source code to library
#
aic_library_sources(${SRCS})