aic_include_directories(inc)

set(SRCS
    ./src/aic_factory_cmd.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
