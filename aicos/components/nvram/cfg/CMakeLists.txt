aic_include_directories(inc)

set(SRCS
    ./src/aic_nvram_device_config.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
