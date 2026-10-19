#/****************************************************************************/
#/*                                                                          */
#/*     Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.              */
#/*                                                                          */
#/****************************************************************************/

cmake_minimum_required(VERSION 3.15)
set(BL_VERSION 1.0.0)

Message("-- Start building ${SUB_TARGET}")
# Some compiler flags depend on the CPU / platform config. This include should
# be run before the toolchain file so the compiler can be configured properly.
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/${TARGET_PROJECT})
    Message(FATAL_ERROR "Unsupported TARGET_PROJECT ${TARGET_PROJECT}/targets/${SUB_TARGET}")
else()
    include(projects/${CMAKE_CONFIG_CHIP}/${TARGET_PROJECT}/targets/${SUB_TARGET}/defconfig.cmake)
	include(projects/${CMAKE_CONFIG_CHIP}/${TARGET_PROJECT}/common/common.cmake)
endif()

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/${TARGET_PROJECT}/common/compiler_options.cmake)
    Message(FATAL_ERROR "Unsupported TARGET_PROJECT ${TARGET_PROJECT}")
else()
    include(projects/${CMAKE_CONFIG_CHIP}/${TARGET_PROJECT}/common/compiler_options.cmake)
endif()

if(NOT ${CMAKE_GENERATOR} STREQUAL "Unix Makefiles" AND
   NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
    Message(FATAL_ERROR "Unsupported generator ${CMAKE_GENERATOR}. Hint: Try -G\"Unix Makefiles\"")
endif()

include(build/toolchain_GNUARM.cmake)

set (CMAKE_C_COMPILER_FORCED true)
# define user/debug version for cmdline
add_compile_options(-DCONFIG_USER_VERSION=${CONFIG_USER_VERSION})
project("Second bootloader" VERSION ${BL_VERSION} LANGUAGES C ASM)

set(DEST_INCLUDE ${AIC_OUT_DIR}/include)
set(BL_PRO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP})

file(COPY ${BL_PRO_DIR}/common/inc/ DESTINATION ${DEST_INCLUDE})
execute_process(
    COMMAND python ${CMAKE_SOURCE_DIR}/build/sign_script/filecopy.py ${BL_PRO_DIR}/${TARGET_PROJECT}/common/config/ ${DEST_INCLUDE}
    COMMAND python ${CMAKE_SOURCE_DIR}/build/sign_script/filecopy.py ${BL_PRO_DIR}/${TARGET_PROJECT}/targets/${SUB_TARGET}/config/ ${DEST_INCLUDE}
)

add_executable(${AIC_BL_NAME})
add_library(boot_arch STATIC)
add_library(boot_driver STATIC)
if (NOT CONFIG_BL_SDK)
    if (CONFIG_DRIVER_CS)
        add_library(boot_driver_cs STATIC)
    endif()
endif()

include_directories(${AIC_OUT_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/common/inc)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/common)
include_directories(${AIC_OUT_DIR}/config)
include_directories(${AIC_PPINLST_DIR})
include_directories(${AIC_PINLST_DIR})
include_directories(${AIC_HWI_DIR})
include_directories(${AIC_TARGET_DIR})

include_directories(
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/common>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/fmd>
)

add_subdirectory(projects)
add_subdirectory(drivers)
add_subdirectory(${AIC_ARCH_DIR})
add_subdirectory(components)

if (NOT CONFIG_BL_SDK)
    add_custom_command(TARGET ${AIC_BL_NAME} PRE_LINK
        COMMAND ${CMAKE_STRIP} -gD ${CMAKE_BINARY_DIR}/*.a
    )
    if (CONFIG_DRIVER_CS)
        add_custom_command(TARGET ${AIC_BL_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libboot_driver_cs.a  ${CMAKE_SOURCE_DIR}/build/libs/${TARGET_PROJECT}/${SUB_TARGET}/
        )
    endif()
endif()
