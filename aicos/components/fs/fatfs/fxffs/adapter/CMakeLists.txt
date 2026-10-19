aic_include_directories(./inc)

set(SRCS
    ./src/fxffs_media_dir.c
    ./src/fxffs_media_file.c
    ./src/fxffs_media_init.c
    ./src/fxffs_media.c
    ./src/fxffs_media_driver.c
    ./src/fxffs_media_unicode.c
    ./src/fxffs.c
    ./src/fxffs_util.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
