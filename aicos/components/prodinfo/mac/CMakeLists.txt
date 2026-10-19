aic_library_include_directories(inc)

set(SRCS
    ./src/mac.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
