
add_subdirectory(adapter)
add_sdk_subdirectory(lfs)
aic_include_directories(./lfs)

if(CONFIG_FS_LFS_IMAGE_GEN)
    set(lfs_bin         ${CMAKE_BINARY_DIR}/bin/aicos_fs.bin)
    set(lfs_size        ${CONFIG_FS_LFS_IMAGE_GEN_SIZE})
    set(lfs_block_size  ${CONFIG_FS_LITTLEFS_BLOCK_SIZE})
    set(lfs_rw_size     ${CONFIG_FS_LITTLEFS_RW_SIZE})
    set(wdir            ${AIC_CHIP_DIR}/fsfile)

    if(EXISTS ${wdir})
        message(STATUS "Gen lfs image from ${wdir} to ${lfs_bin}[${lfs_size} ${lfs_block_size} ${lfs_rw_size}]")

        if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
            set(littlefs_tool    ${AIC_ORIGIN_ROOT}/build/tools/lfs/mklittlefs.exe)
        elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
            set(littlefs_tool    ${AIC_ORIGIN_ROOT}/build/tools/lfs/mklittlefs)
            execute_process(
                COMMAND             chmod a+x ${littlefs_tool}
                TIMEOUT             60
                RESULT_VARIABLE     result
                ERROR_VARIABLE      output
                OUTPUT_VARIABLE     output
            )
            if(NOT ${result} EQUAL 0)
                message(FATAL_ERROR "Could not chmod a+x ${littlefs_tool}")
            endif()
        else()
            message(FATAL_ERROR "Unknown ${CMAKE_HOST_SYSTEM_NAME} operating system environment")
        endif()

        execute_process(
            WORKING_DIRECTORY   ${wdir}
            COMMAND             ${littlefs_tool} -c ${wdir} -a -b ${lfs_block_size} -p ${lfs_rw_size} -s ${lfs_size} ${lfs_bin}
            TIMEOUT             60
            RESULT_VARIABLE     result
            ERROR_VARIABLE      output
            OUTPUT_VARIABLE     output
        )

        if(NOT ${result} EQUAL 0)
            message(FATAL_ERROR "Could not Gen lfs ${lfs_bin} --> ${output}")
        endif()

    else()
        message(WARNING "LFS GEN: ${wdir} not exist!")
    endif()
endif()
