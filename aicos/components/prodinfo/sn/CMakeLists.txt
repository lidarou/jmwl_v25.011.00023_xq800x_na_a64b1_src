aic_library_include_directories(inc)

set(SRCS
    ./src/sn.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
