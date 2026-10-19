
target_sources(boot_driver
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/fxffs_media_dir.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fxffs_media_init.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fxffs_media.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fxffs_media_driver.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fxffs_media_file.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fxffs.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fxffs_util.c
)


target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)
