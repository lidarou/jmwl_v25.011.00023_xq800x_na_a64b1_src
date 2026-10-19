aic_include_directories(inc)

set(SRCS
    ./src/vfs_blkdev_register.c
    ./src/vfs_check.c
    ./src/vfs_config.c
    ./src/vfs_cwd.c
    ./src/vfs_errno.c
    ./src/vfs_fd.c
    ./src/vfs_fs.c
    ./src/vfs_init.c
    ./src/vfs_inode.c
    ./src/vfs_mount.c
    ./src/vfs_os.c
    ./src/vfs_type.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
