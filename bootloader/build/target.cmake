#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

cmake_policy(SET CMP0076 NEW)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

if (NOT CONFIG_BL_SDK)
    if (CONFIG_DRIVER_CS)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/build/libs/${TARGET_PROJECT}/${SUB_TARGET}
        )
    endif()
endif()

target_link_libraries(${AIC_BL_NAME}
    PUBLIC
        boot_arch
        boot_driver
)

if (NOT CONFIG_BL_SDK)
    if (CONFIG_DRIVER_CS)
        target_link_libraries(${AIC_BL_NAME}
             PUBLIC
                 boot_driver_cs
        )
    endif()
else()
    if (CONFIG_DRIVER_CS)
        target_link_libraries(${AIC_BL_NAME}
             PUBLIC
                 "-Wl,--whole-archive"
                 "${CMAKE_SOURCE_DIR}/build/libs/${TARGET_PROJECT}/${SUB_TARGET}/libboot_driver_cs.a"
                 "-Wl,--no-whole-archive"
        )
    endif()
endif()

set_target_properties(${AIC_BL_NAME}
    PROPERTIES
        SUFFIX ".elf"
        RUNTIME_OUTPUT_DIRECTORY "${AIC_BL_BINARY_DIR}/bin"
)

target_link_options(${AIC_BL_NAME}
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wl,-Map=${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}.map>
)

add_convert_to_bin_target(${AIC_BL_NAME})

if (CONFIG_AIC_SIGNATURE)
    find_package(Python3)

    add_custom_target(${AIC_BL_NAME}_signed_bin
        SOURCES ${AIC_BL_NAME}_signed.bin
    )

    if (CONFIG_SECURE_BOOT)
    if (CONFIG_FOTA_UPGRADE)
        if (CONFIG_COMPRESSED_BIN)
            add_custom_command(OUTPUT ${AIC_BL_NAME}_signed.bin
                DEPENDS $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sign_script

                #Sign secure binary image with provided secret key
                COMMAND python -B ${CMAKE_SOURCE_DIR}/build/sign_script/imgsign.py
                    -m ${AIC_HEADER_MAGIC_NUMBER}
                    -s ${AIC_SW_VERISON}
                    -k ${AIC_SIGN_KEY_DIR}/bl_priv_key.pem
                    -pub ${AIC_SIGN_KEY_DIR}/bl_pub_key.pem
                    --keysign
                    -nk ${AIC_SIGN_KEY_DIR}/bl_pub_key.pem
                    $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                    ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin
                    --fota
                COMMAND ${CMAKE_COMMAND} -E copy ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin $<TARGET_FILE_DIR:${AIC_BL_NAME}>
                COMMAND python ${AIC_BUILD}/compression_proc.py ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed_compressed.bin
            )
        else()
            add_custom_command(OUTPUT ${AIC_BL_NAME}_signed.bin
                DEPENDS $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sign_script

                #Sign secure binary image with provided secret key
                COMMAND python -B ${CMAKE_SOURCE_DIR}/build/sign_script/imgsign.py
                    -m ${AIC_HEADER_MAGIC_NUMBER}
                    -s ${AIC_SW_VERISON}
                    -k ${AIC_SIGN_KEY_DIR}/bl_priv_key.pem
                    -pub ${AIC_SIGN_KEY_DIR}/bl_pub_key.pem
                    --keysign
                    -nk ${AIC_SIGN_KEY_DIR}/bl_pub_key.pem
                    $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                    ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin
                    --fota
                COMMAND ${CMAKE_COMMAND} -E copy ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin $<TARGET_FILE_DIR:${AIC_BL_NAME}>
            )
        endif()
    else()
        if (CONFIG_COMPRESSED_BIN)
            add_custom_command(OUTPUT ${AIC_BL_NAME}_signed.bin
                DEPENDS $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sign_script

                #Sign secure binary image with provided secret key
                COMMAND python -B ${CMAKE_SOURCE_DIR}/build/sign_script/imgsign.py
                    -m ${AIC_HEADER_MAGIC_NUMBER}
                    -s ${AIC_SW_VERISON}
                    -k ${AIC_SIGN_KEY_DIR}/bl_priv_key.pem
                    -pub ${AIC_SIGN_KEY_DIR}/bl_pub_key.pem
                    --keysign
                    -nk ${AIC_SIGN_KEY_DIR}/bl_pub_key.pem
                    $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                    ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin
                COMMAND ${CMAKE_COMMAND} -E copy ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin $<TARGET_FILE_DIR:${AIC_BL_NAME}>
                COMMAND python ${AIC_BUILD}/compression_proc.py ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed_compressed.bin
            )
        else()
            add_custom_command(OUTPUT ${AIC_BL_NAME}_signed.bin
                DEPENDS $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sign_script

                #Sign secure binary image with provided secret key
                COMMAND python -B ${CMAKE_SOURCE_DIR}/build/sign_script/imgsign.py
                    -m ${AIC_HEADER_MAGIC_NUMBER}
                    -s ${AIC_SW_VERISON}
                    -k ${AIC_SIGN_KEY_DIR}/bl_priv_key.pem
                    -pub ${AIC_SIGN_KEY_DIR}/bl_pub_key.pem
                    --keysign
                    -nk ${AIC_SIGN_KEY_DIR}/bl_pub_key.pem
                    $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                    ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin
                COMMAND ${CMAKE_COMMAND} -E copy ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin $<TARGET_FILE_DIR:${AIC_BL_NAME}>
            )
            endif()
        endif()
    else()
        if (CONFIG_FOTA_UPGRADE)
            if (CONFIG_COMPRESSED_BIN)
                add_custom_command(OUTPUT ${AIC_BL_NAME}_signed.bin
                    DEPENDS $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sign_script
                    #only add header
                    COMMAND python -B ${CMAKE_SOURCE_DIR}/build/sign_script/addHeader.py
                        -i $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                        -o ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin
                        --fota
                    COMMAND ${CMAKE_COMMAND} -E copy ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin $<TARGET_FILE_DIR:${AIC_BL_NAME}>
                    COMMAND python ${AIC_BUILD}/compression_proc.py ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed_compressed.bin
                )
            else()
                add_custom_command(OUTPUT ${AIC_BL_NAME}_signed.bin
                    DEPENDS $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sign_script
                    #only add header
                    COMMAND python -B ${CMAKE_SOURCE_DIR}/build/sign_script/addHeader.py
                        -i $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                        -o ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin
                        --fota
                    COMMAND ${CMAKE_COMMAND} -E copy ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin $<TARGET_FILE_DIR:${AIC_BL_NAME}>
                )
            endif()
        else()
            if (CONFIG_COMPRESSED_BIN)
                add_custom_command(OUTPUT ${AIC_BL_NAME}_signed.bin
                    DEPENDS $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sign_script
                    #only add header
                    COMMAND python -B ${CMAKE_SOURCE_DIR}/build/sign_script/addHeader.py
                        -i $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                        -o ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin
                    COMMAND ${CMAKE_COMMAND} -E copy ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin $<TARGET_FILE_DIR:${AIC_BL_NAME}>
                    COMMAND python ${AIC_BUILD}/compression_proc.py ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed_compressed.bin
                )
            else()
                add_custom_command(OUTPUT ${AIC_BL_NAME}_signed.bin
                    DEPENDS $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sign_script
                    #only add header
                    COMMAND python -B ${CMAKE_SOURCE_DIR}/build/sign_script/addHeader.py
                        -i $<TARGET_FILE_DIR:${AIC_BL_NAME}>/${AIC_BL_NAME}.bin
                        -o ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin
                    COMMAND ${CMAKE_COMMAND} -E copy ${AIC_BL_BINARY_DIR}/bin/${AIC_BL_NAME}_signed.bin $<TARGET_FILE_DIR:${AIC_BL_NAME}>
                )
            endif()
        endif()
    endif()

    add_custom_target(bootloader_signed_images
        ALL
        DEPENDS ${AIC_BL_NAME}_signed_bin
    )
endif()