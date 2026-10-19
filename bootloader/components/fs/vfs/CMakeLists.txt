target_sources(boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_blkdev_register.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_check.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_config.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_errno.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_fd.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_fs.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_init.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_inode.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_mount.c
    ${CMAKE_CURRENT_LIST_DIR}/src/vfs_type.c
)


target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)
