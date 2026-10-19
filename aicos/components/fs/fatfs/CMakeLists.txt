
add_subdirectory_ifdef(CONFIG_AZURE_FILEX fxffs)

if(CONFIG_FS_FATFS_IMAGE_GEN)
    set(fat_bin     ${CMAKE_BINARY_DIR}/bin/aicos_fs.bin)
    set(fat_size    ${CONFIG_FS_FATFS_IMAGE_GEN_SIZE})
    set(fat_format  fat12)
    set(wdir        ${AIC_CHIP_DIR}/fsfile)

    if(EXISTS ${wdir})
        message(STATUS "Gen fatfs image from ${wdir} to ${fat_bin}[${fat_size} ${fat_format}]")

        if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
            set(fatdisk_tool ${AIC_ORIGIN_ROOT}/build/tools/fatfs/fatdisk.exe)
        elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
            set(fatdisk_tool ${AIC_ORIGIN_ROOT}/build/tools/fatfs/fatdisk)
            execute_process(
                COMMAND             chmod a+x ${fatdisk_tool}
                TIMEOUT             60
                RESULT_VARIABLE     result
                ERROR_VARIABLE      output
                OUTPUT_VARIABLE     output
            )
            if(NOT ${result} EQUAL 0)
                message(FATAL_ERROR "Could not chmod a+x ${fatdisk_tool}")
            endif()
        else()
            message(FATAL_ERROR "Unknown ${CMAKE_HOST_SYSTEM_NAME} operating system environment")
        endif()

        execute_process(
            WORKING_DIRECTORY   ${wdir}
            COMMAND             ${fatdisk_tool} ${fat_bin} format size ${fat_size} name "aicfatfs" ${fat_format}
            TIMEOUT             60
            RESULT_VARIABLE     result
            ERROR_VARIABLE      output
            OUTPUT_VARIABLE     output
        )

        if(NOT ${result} EQUAL 0)
            message(FATAL_ERROR "Could not format ${fat_bin} --> ${output}")
        endif()

        execute_process(
            WORKING_DIRECTORY   ${wdir}
            COMMAND             ${fatdisk_tool} ${fat_bin} add *
            TIMEOUT             60
            RESULT_VARIABLE     result
            ERROR_VARIABLE      output
            OUTPUT_VARIABLE     output
        )

        if(NOT ${result} EQUAL 0)
            message(FATAL_ERROR "Pack failed --> ${output}")
        endif()
    else()
        message(WARNING "FATFS GEN: ${wdir} not exist!")
    endif()
endif()
