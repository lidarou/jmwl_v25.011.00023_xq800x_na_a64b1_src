
target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)

add_subdirectory(dev)
add_subdirectory(vfs)
add_subdirectory_ifdef(CONFIG_FS_FATFS fatfs)
