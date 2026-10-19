cmake_minimum_required(VERSION 3.13)

# Use the new CMake behaviour until we are updating the CMake 3.21 as minimum
# required. This ensure that CMake >=3.21 will be consistent with new CMakes.
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.21)
    cmake_policy(SET CMP0123 NEW)
endif()

#
# Call compile
#
include(${CMAKE_SCRIPT_PATH}/cmake/compile.cmake)

#
# Create boards
#
set(BUILD_SHARED_LIBS OFF)
set(project_version "0.0.1")
project(${CMAKE_CONFIG_BOARD} VERSION ${project_version})
enable_language(C CXX ASM)

#
# The setup / configuration of the toolchain itself and the configuration of
# supported compilation flags are now split, as this allows to use the toolchain
# for generic purposes.
# Testing the toolchain flags requires the enable_language() to have been called in CMake.
#
include(${CMAKE_SCRIPT_PATH}/cmake/target_toolchain_flags.cmake)
include(${CMAKE_SCRIPT_PATH}/cmake/compile_flag_config.cmake)

#
# KConfig build
# If it is KConfig build, return and do not add executable project
#
if (CMAKE_KCONFIG_BUILD)
    set(CMAKE_KCONFIG_BUILD "False" CACHE BOOL INTERNAL FORCE)
    return()
endif()

# Set CMAKE_C_CREATE_STATIC_LIBRARY to use -D
if (CONFIG_SUBSYS_AP)
  set(CMAKE_C_CREATE_STATIC_LIBRARY
    "<CMAKE_AR> qD <TARGET> <LINK_FLAGS> <OBJECTS>"
    "<CMAKE_RANLIB> -D <TARGET>")
endif()
#
# FIXME: Is there any way to get rid of empty_file.c?
#
add_executable(${PROJECT_NAME} ${CMAKE_SCRIPT_PATH}/empty_file.c)
set(CMAKE_EXECUTABLE_SUFFIX .elf)

#
# include sub-dircetory's CMakeLists.txt
# add commom include directorys.
#
include_directories(components/include)
include_directories(components/include/cmsis)
include_directories(components/include/module)
include_directories(components/include/at)
include_directories(drivers/include)
include_directories(os/osa/common/inc)
include_directories(os/osa/include)
include_directories(os/osa/bounds_checking_function/include)
include_directories(os/kernel/threadx/common/inc)
include_directories(${AIC_PPINLST_DIR})
include_directories(${AIC_PINLST_DIR})
include_directories(${AIC_HWI_DIR})
include_directories(${AIC_PMIC_DIR})
include_directories(${AIC_TARGET_DIR})
include_directories(${AIC_CHIP_COM_DIR})
include_directories(${AIC_CHIP_DIR})
include_directories(${AIC_CHIP_DIR}/chip_reg)
include_directories(${AIC_CHIP_DIR}/aipc)
include_directories(${AIC_CHIP_DIR}/alog)
include_directories(${AIC_CFG_PATH}/config)
include_directories(${AIC_OUT_DIR}/config)
include_directories(${AIC_NVRAM_COM_DIR})
include_directories(${AIC_NVRAM_CFG_PATH})
include_directories(${AIC_NVRAM_CFG_PATH}/config)
add_subdirectory(drivers)
add_subdirectory(os/osa)
if(CONFIG_OS_SYSTEM STREQUAL "azure-rtos")
add_subdirectory(os/kernel/threadx)
endif()
if(CONFIG_OS_SYSTEM STREQUAL "liteos_m")
add_subdirectory(os/kernel/liteos_m)
endif()
add_subdirectory(${AIC_CFG_PATH})
add_subdirectory(components)
add_subdirectory_ifdef(CONFIG_SUBSYS_AP         vendors)
add_subdirectory_ifdef(CONFIG_MM                mm)
add_subdirectory_ifdef(CONFIG_APP               app)
add_sdk_subdirectory_ifdef(CONFIG_COMM_IMS      comm/ims)
add_sdk_subdirectory_ifdef(CONFIG_COMM_PS       comm/ps)
add_sdk_subdirectory_ifdef(CONFIG_COMM_PHY      comm/phy)
aic_include_directories_ifdef(CONFIG_SUBSYS_CP  comm/rfnv/include)

#
# Set the last librarise.
# Some libraries could not be set in whole-archive, but their should
# be set in order, please set them here.
#
aic_append_toolchain_library(gcc)

# Read list content
get_property(AIC_LIBS_PROPERTY      GLOBAL  PROPERTY AIC_LIBS)
get_property(AIC_EXP_LIBS_PROPERTY  GLOBAL  PROPERTY AIC_EXPORT_LIBS)
get_property(AIC_EXT_LIBS_PROPERTY  GLOBAL  PROPERTY AIC_EXTERNAL_LIBS)
get_property(TOOLCHAIN_LIBS GLOBAL  PROPERTY    TOOLCHAIN_LIBS)

foreach(aic_lib ${AIC_LIBS_PROPERTY})
    get_property(lib_type TARGET ${aic_lib} PROPERTY TYPE)

    # To prevent CMake failure when a driver is enabled, for example: REGULATOR=y
    # we disable any Zephyr libraries without sources and adds the `empty_file.c`.
    if(${lib_type} STREQUAL STATIC_LIBRARY)
        get_property(source_list  TARGET ${aic_lib} PROPERTY SOURCES)
        get_property(lib_imported TARGET ${aic_lib} PROPERTY IMPORTED)

        if(NOT source_list AND NOT ${lib_imported})
            message(WARNING
            "No SOURCES given to AIC library: ${aic_lib}\nExcluding target from build."
            )
            target_sources(${aic_lib} PRIVATE ${CMAKE_SCRIPT_PATH}/empty_file.c)
            set_property(TARGET ${aic_lib} PROPERTY EXCLUDE_FROM_ALL TRUE)
            list(REMOVE_ITEM AIC_LIBS_PROPERTY ${aic_lib})
            continue()
        endif()
    endif()

    # TODO: Could this become an INTERFACE property of aic_interface?
    #add_dependencies(${aic_lib} aic_generated_headers)
endforeach()

if (NOT "${COMPILER}" STREQUAL "clang")
    toolchain_ld_link_elf(
        TARGET_ELF            ${PROJECT_NAME}
        OUTPUT_MAP            ${PROJECT_BINARY_DIR}/aicos_${CMAKE_SUBSYS_CONFIG}.map
        LIBRARIES_PRE_SCRIPT  ""
        LINKER_SCRIPT         ${AIC_BUILD_ROOT}/aic_board.ld
        DEPENDENCIES          ${CODE_RELOCATION_DEP}
    )

    if (${CMAKE_SUBSYS_CONFIG} STREQUAL "cp")
        toolchain_objcopy_multi_binary(
            ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
            ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
            ram1
        )
        toolchain_objcopy_multi_binary(
            ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
            ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin
            ram2
        )

        if (CONFIG_MCX_SUPPORT)
            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash.bin
                flash
            )
        endif()

        if (CONFIG_SUPPORT_4_PSRAM_BIN)
            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                ram3
            )
            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin
                ram4
            )
        endif()

        if (CONFIG_SUPPORT_6_PSRAM_BIN)
            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                ram5
            )
            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin
                ram6
            )
        endif()

        if (CONFIG_GEN_TCM_BIN)
            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_atcm.bin
                atcm
            )
            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_btcm.bin
                btcm
            )
        endif()

        if (${CMAKE_SECURE_BOARD} STREQUAL "NSEC")
            add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.elf                ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.elf
                COMMAND ${CMAKE_COMMAND} -E copy aicos_${CMAKE_SUBSYS_CONFIG}.map   ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
                COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_NAME}.elf aicos_${CMAKE_SUBSYS_CONFIG}.map ${PROJECT_NAME}.elf.map
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin

                #compress bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/map_analyse.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
            )

            if (CONFIG_MCX_SUPPORT)
                add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash.bin
                )
            endif()

            if (CONFIG_SUPPORT_4_PSRAM_BIN)
                add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin
                    #Compress bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin
                )
            endif()

            if (CONFIG_SUPPORT_6_PSRAM_BIN)
                add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin
                    #Compress bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin
                )
            endif()

            if (CONFIG_GEN_TCM_BIN)
                add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_atcm.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_btcm.bin
                    #Compress bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_atcm.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_btcm.bin
                )
            endif()
        else()
            add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.elf                ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.elf
                COMMAND ${CMAKE_COMMAND} -E copy aicos_${CMAKE_SUBSYS_CONFIG}.map   ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
                COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_NAME}.elf aicos_${CMAKE_SUBSYS_CONFIG}.map ${PROJECT_NAME}.elf.map
                COMMAND python ${CMAKE_SCRIPT_PATH}/map_analyse.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
            )
            if (CONFIG_MCX_SUPPORT)
                add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash_signed.bin
                )
            endif()

            if (CONFIG_SUPPORT_4_PSRAM_BIN)
                add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3_signed.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4_signed.bin

                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3_signed.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4_signed.bin
                )
            endif()

            if (CONFIG_SUPPORT_6_PSRAM_BIN)
                add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5_signed.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6_signed.bin

                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5_signed.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6_signed.bin
                )
            endif()

            if (CONFIG_GEN_TCM_BIN)
                add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_atcm.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_atcm_signed.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_btcm.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_btcm_signed.bin

                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_atcm_signed.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_btcm_signed.bin
                )
            endif()
            add_custom_command(TARGET ${PROJECT_NAME}
                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1_signed.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2_signed.bin

                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1_signed.bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2_signed.bin
                    )
        endif()
    else()
        if (CONFIG_MULTI_BIN_SUPPORT)
            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_ap.bin
                basic
            )

            toolchain_objcopy_multi_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/bin/aicos_ap_extend.bin
                extend
            )

            if (CONFIG_COMPRESS_EXTEND_AP)
                add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.elf                ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.elf
                    COMMAND ${CMAKE_COMMAND} -E copy aicos_${CMAKE_SUBSYS_CONFIG}.map   ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
                    COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_NAME}.elf aicos_${CMAKE_SUBSYS_CONFIG}.map
                    COMMAND python ${CMAKE_SCRIPT_PATH}/map_analyse.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
                    COMMAND python ${CMAKE_SCRIPT_PATH}/add_check_sum.py ./bin/aicos_ap_extend.bin 0 ./bin/aicos_ap.bin
                    #compress bin
                    COMMAND python ${CMAKE_SCRIPT_PATH}/compress_image_with_header.py ./bin/aicos_ap_extend.bin 20
                )
            else()
                add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.elf                ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.elf
                    COMMAND ${CMAKE_COMMAND} -E copy aicos_${CMAKE_SUBSYS_CONFIG}.map   ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
                    COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_NAME}.elf aicos_${CMAKE_SUBSYS_CONFIG}.map
                    COMMAND python ${CMAKE_SCRIPT_PATH}/map_analyse.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
                    COMMAND python ${CMAKE_SCRIPT_PATH}/add_check_sum.py ./bin/aicos_ap_extend.bin 0 ./bin/aicos_ap.bin
                )
            endif()
        else()
            toolchain_objcopy_binary(
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
                ${PROJECT_BINARY_DIR}/aicos_${CMAKE_SUBSYS_CONFIG}.bin
            )

            add_custom_command(TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.elf                ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.elf
                COMMAND ${CMAKE_COMMAND} -E copy aicos_${CMAKE_SUBSYS_CONFIG}.map   ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
                COMMAND ${CMAKE_COMMAND} -E copy aicos_${CMAKE_SUBSYS_CONFIG}.bin   ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_NAME}.elf aicos_${CMAKE_SUBSYS_CONFIG}.bin aicos_${CMAKE_SUBSYS_CONFIG}.map
                COMMAND python ${CMAKE_SCRIPT_PATH}/map_analyse.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
            )
        endif()

        if (${CMAKE_SECURE_BOARD} STREQUAL "SEC")
            if (CONFIG_MULTI_BIN_SUPPORT)
                add_custom_command(TARGET ${PROJECT_NAME}
                    POST_BUILD

                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        --keysign
                        -nk ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_signed.bin

                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        --keysign
                        -nk ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_ap_extend.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_ap_extend_signed.bin
                )
            else()
                add_custom_command(TARGET ${PROJECT_NAME}
                    POST_BUILD

                    #Sign secure binary image with provided secret key
                    COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                        -m 0x55aa7e7e
                        -s ${CMAKE_SW_VERISON}
                        -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                        -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        --keysign
                        -nk ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}.bin
                        ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_signed.bin
                )
            endif()
        endif()
    endif()
else()
    toolchain_ld_link_elf(
        TARGET_ELF            ${PROJECT_NAME}
        OUTPUT_MAP            ${PROJECT_BINARY_DIR}/aicos_${CMAKE_SUBSYS_CONFIG}.map
        LIBRARIES_PRE_SCRIPT  ""
        LINKER_SCRIPT         ${AIC_BUILD_ROOT}/aic_board.scat
        DEPENDENCIES          ${CODE_RELOCATION_DEP}
    )

    toolchain_objcopy_binary(
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
        ${PROJECT_BINARY_DIR}/
    )

    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.elf                ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.elf
        COMMAND ${CMAKE_COMMAND} -E copy aicos_${CMAKE_SUBSYS_CONFIG}.map   ./bin/aicos_${CMAKE_SUBSYS_CONFIG}.map
        COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_NAME}.elf aicos_${CMAKE_SUBSYS_CONFIG}.map ${PROJECT_NAME}.elf.map
    )

    if (${CMAKE_SECURE_BOARD} STREQUAL "NSEC")
        if (CONFIG_MCX_SUPPORT)
            add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy MCX_CODE ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f MCX_CODE
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash.bin
            )
        endif()

        if (CONFIG_SUPPORT_4_PSRAM_BIN)
            add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy CODE_3 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                COMMAND ${CMAKE_COMMAND} -E rm CODE_3
                COMMAND ${CMAKE_COMMAND} -E copy CODE_4 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin
                COMMAND ${CMAKE_COMMAND} -E rm CODE_4
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin

                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin
            )
        endif()

        if (CONFIG_SUPPORT_6_PSRAM_BIN)
            add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy CODE_5 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                COMMAND ${CMAKE_COMMAND} -E rm CODE_5
                COMMAND ${CMAKE_COMMAND} -E copy CODE_6 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin
                COMMAND ${CMAKE_COMMAND} -E rm CODE_6
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin

                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin
            )
        endif()

        add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy TCM ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
                COMMAND ${CMAKE_COMMAND} -E copy CODE_2 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f TCM CODE_2
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/addHeader.py
                                -i ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin

                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin
        )
    else()
        if (CONFIG_MCX_SUPPORT)
            add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy MCX_CODE ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f MCX_CODE
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                    -m 0x55aa7e7e
                    -s ${CMAKE_SW_VERISON}
                    -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                    -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash.bin
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_flash_signed.bin
            )
        endif()

        if (CONFIG_SUPPORT_4_PSRAM_BIN)
            add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy CODE_3 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f CODE_3
                COMMAND ${CMAKE_COMMAND} -E copy CODE_4 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f CODE_4
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                    -m 0x55aa7e7e
                    -s ${CMAKE_SW_VERISON}
                    -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                    -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3.bin
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3_signed.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                    -m 0x55aa7e7e
                    -s ${CMAKE_SW_VERISON}
                    -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                    -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4.bin
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4_signed.bin

                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram3_signed.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram4_signed.bin
            )
        endif()

        if (CONFIG_SUPPORT_6_PSRAM_BIN)
            add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy CODE_5 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f CODE_5
                COMMAND ${CMAKE_COMMAND} -E copy CODE_6 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f CODE_6
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                    -m 0x55aa7e7e
                    -s ${CMAKE_SW_VERISON}
                    -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                    -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5.bin
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5_signed.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                    -m 0x55aa7e7e
                    -s ${CMAKE_SW_VERISON}
                    -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                    -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6.bin
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6_signed.bin

                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram5_signed.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram6_signed.bin
            )
        endif()

        add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy TCM ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
                COMMAND ${CMAKE_COMMAND} -E copy CODE_2 ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin
                COMMAND ${CMAKE_COMMAND} -E rm -f TCM CODE_2
                #Sign secure binary image with provided secret key
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                    -m 0x55aa7e7e
                    -s ${CMAKE_SW_VERISON}
                    -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                    -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1.bin
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1_signed.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/sign_script/imgsign.py
                    -m 0x55aa7e7e
                    -s ${CMAKE_SW_VERISON}
                    -k ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_priv_key.pem
                    -pub ${AIC_TARGET_DIR}/../${CMAKE_CONFIG_PROJECT}/key/bl_pub_key.pem
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2.bin
                    ${PROJECT_BINARY_DIR}/bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2_signed.bin

                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram1_signed.bin
                COMMAND python ${CMAKE_SCRIPT_PATH}/compression_proc.py ./bin/aicos_${CMAKE_SUBSYS_CONFIG}_ram2_signed.bin
                )
    endif()
endif()
