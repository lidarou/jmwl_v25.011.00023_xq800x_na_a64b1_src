aic_include_directories(inc)

set(SRCS
    ./src/aic_md_cfg.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
