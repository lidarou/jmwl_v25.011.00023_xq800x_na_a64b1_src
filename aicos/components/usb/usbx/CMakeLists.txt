cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(usbx
    LANGUAGES C ASM
)    

string(REPLACE "-" "_" THREADX_ARCH ${CONFIG_CPU_NAME})
if ("${COMPILER}" STREQUAL "gcc")
    set(THREADX_TOOLCHAIN "gnu")
else()
    set(THREADX_TOOLCHAIN ${COMPILER})
endif()
if ("${THREADX_ARCH}" STREQUAL "")
    message(FATAL_ERROR "Error: THREADX_ARCH not defined")
endif()
if ("${THREADX_TOOLCHAIN}" STREQUAL "")
    message(FATAL_ERROR "Error: THREADX_TOOLCHAIN not defined")
endif()

# Define our target library and an alias for consumers
# aic_sdk_library is register close source code library
aic_sdk_library(${PROJECT_NAME})
    if(NOT CONFIG_SDK_ENABLE)

    add_library("azrtos::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

    # Define any required dependencies between this library and others
    target_link_libraries(${PROJECT_NAME} PUBLIC
        "azrtos::threadx"
    )

    # A place for generated/copied include files
    set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

    # Include USBX configurations for compiler
    aic_compile_options("SHELL: $<TARGET_PROPERTY:compiler,imacros> ${AIC_CFG_PATH}/config/ux_config.h")

    # Pick up the port specific stuff first
    if(DEFINED USBX_CUSTOM_PORT)
        add_subdirectory(${USBX_CUSTOM_PORT} usbx_port)
    else()
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN})
    endif()

    # Then the common files
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/common)

    # If the user provided an override, copy it to the custom directory
    if (NOT UX_USER_FILE)
        message(STATUS "Using default ux_user.h file")
        set(UX_USER_FILE ${CMAKE_CURRENT_LIST_DIR}/common/core/inc/ux_user_sample.h)
    else()
        message(STATUS "Using custom ux_user.h file from ${UX_USER_FILE}")
    endif()
    configure_file(${UX_USER_FILE} ${CUSTOM_INC_DIR}/ux_user.h COPYONLY)
    target_include_directories(${PROJECT_NAME} 
        PUBLIC 
        ${CUSTOM_INC_DIR}
    )
    target_compile_definitions(${PROJECT_NAME} PUBLIC "UX_INCLUDE_USER_DEFINE_FILE" )

else()
    # A place for generated/copied include files
    set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

    # Include USBX configurations for compiler
    aic_compile_options("SHELL: $<TARGET_PROPERTY:compiler,imacros> ${AIC_CFG_PATH}/config/ux_config.h")

    # If the user provided an override, copy it to the custom directory
    if (NOT UX_USER_FILE)
        message(STATUS "Using default ux_user.h file")
        set(UX_USER_FILE ${CMAKE_CURRENT_LIST_DIR}/common/core/inc/ux_user_sample.h)
    else()
        message(STATUS "Using custom ux_user.h file from ${UX_USER_FILE}")
    endif()
    configure_file(${UX_USER_FILE} ${CUSTOM_INC_DIR}/ux_user.h COPYONLY)

    aic_compile_definitions("UX_INCLUDE_USER_DEFINE_FILE")

    aic_include_directories(
        ${CUSTOM_INC_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/common/core/inc
        ${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN}/inc
        ${CMAKE_CURRENT_LIST_DIR}/common/usbx_network/inc
        ${CMAKE_CURRENT_LIST_DIR}/common/usbx_device_classes/inc
    )

endif()

# Enable a build target that produces a ZIP file of all sources
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES
  \\.git/
  \\.github/
  _build/
  \\.git
  \\.gitattributes
  \\.gitignore
  ".*~$"
)
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)
