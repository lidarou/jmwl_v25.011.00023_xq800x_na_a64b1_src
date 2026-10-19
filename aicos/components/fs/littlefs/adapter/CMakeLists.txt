aic_include_directories(inc)

set(SRCS
    ./src/aic_lfs.c
    ./src/aic_lfs_bd.c
    ./src/aic_lfs_util.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
