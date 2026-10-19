cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(aic_fs
    VERSION 1.0.0
    LANGUAGES C
)

#
# Create Library and add Curent Library to project library list
# aic_library_named is register open source code library
#
aic_library_named(${PROJECT_NAME})

#############################################################
# Public curent header file to total project
# Add the directory to INTERFACE LIBRARY
#############################################################
#
aic_include_directories_ifdef(CONFIG_FS
    ./inc
)

add_subdirectory_ifdef(CONFIG_FS dev)
add_subdirectory_ifdef(CONFIG_FS vfs)
add_subdirectory_ifdef(CONFIG_FS_FATFS fatfs)
add_sdk_subdirectory_ifdef(CONFIG_FS_ROMFS romfs)
add_subdirectory_ifdef(CONFIG_FS_LITTLEFS littlefs)
add_subdirectory_ifdef(CONFIG_FS at)
add_subdirectory_ifdef(CONFIG_FS test)

if(CONFIG_FS_ROMFS)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(romfs_tool    ${AIC_ORIGIN_ROOT}/build/tools/romfs/genromfs.exe)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(romfs_tool    ${AIC_ORIGIN_ROOT}/build/tools/romfs/genromfs)
    execute_process(
        COMMAND             chmod a+x ${romfs_tool}
        TIMEOUT             60
        RESULT_VARIABLE     result
        ERROR_VARIABLE      output
        OUTPUT_VARIABLE     output
    )
    if(NOT ${result} EQUAL 0)
        message(FATAL_ERROR "Could not chmod a+x ${romfs_tool}")
    endif()
else()
    message(FATAL_ERROR "Unknown ${CMAKE_HOST_SYSTEM_NAME} operating system environment")
endif()

execute_process(
    COMMAND ${romfs_tool} -V "aicxtekromfs" -f ${CMAKE_BINARY_DIR}/bin/aicos_romfs.bin -d ${AIC_CHIP_DIR}/romfs
)

add_custom_command(
    OUTPUT
        romfs_checksum
    COMMAND python ${CMAKE_SCRIPT_PATH}/add_check_sum.py ${CMAKE_BINARY_DIR}/bin/aicos_romfs.bin
    DEPENDS
        ${CMAKE_SCRIPT_PATH}/add_check_sum.py
)

add_custom_target(romfs_force_check ALL DEPENDS romfs_checksum)
endif()
