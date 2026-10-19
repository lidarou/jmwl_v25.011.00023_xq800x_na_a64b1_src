
target_sources_ifdef(CONFIG_FS_DEV_FLASH boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/fs_dev_ftl.c
)
