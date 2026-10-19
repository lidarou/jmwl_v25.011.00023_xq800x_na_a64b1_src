aic_library_include_directories(inc)

set(SRCS
    ./src/fs_at_common.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_sources_ifdef(CONFIG_FS_AT
    ./src/fs_gf_at.c
    ./src/fs_crc16.c
)

aic_library_sources_ifdef(CONFIG_FS_CUSTOM_AT
    ./src/fs_qf_at.c
)

aic_library_sources_ifdef(CONFIG_BLT_SUPPORT
    ./src/fs_blt_at.c
)
