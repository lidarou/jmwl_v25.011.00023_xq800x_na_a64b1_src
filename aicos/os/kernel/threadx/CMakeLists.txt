cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(threadx
    VERSION 6.0.0
    LANGUAGES C ASM
)

string(REPLACE "-" "_" THREADX_ARCH ${CONFIG_CPU_NAME})
set(THREADX_TOOLCHAIN   ${CONFIG_COMPILER_NAME})

if ("${THREADX_ARCH}" STREQUAL "")
    message(FATAL_ERROR "Error: THREADX_ARCH not defined")
endif()
if ("${THREADX_TOOLCHAIN}" STREQUAL "")
    message(FATAL_ERROR "Error: THREADX_TOOLCHAIN not defined")
endif()

# Define our target library and an alias for consumers
# aic_sdk_library is register close source code library
aic_export_sdk_library(${PROJECT_NAME})
if(NOT CONFIG_SDK_ENABLE)

    add_library("azrtos::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

    # A place for generated/copied include files (no need to change)
    set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

    # Pick up the port specific variables and apply them
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN})
    target_include_directories(aic_interface
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN}/inc
        )

    # Pick up the common stuff
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/common)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/utility/rtos_compatibility_layers)


    # If the user provided an override, copy it to the custom directory
    if (NOT TX_USER_FILE)
        message(STATUS "Using default tx_user.h file")
        set(TX_USER_FILE ${AIC_CFG_PATH}/config/tx_user.h)
    else()
        message(STATUS "Using custom tx_user.h file from ${TX_USER_FILE}")
    endif()
    configure_file(${TX_USER_FILE} ${CUSTOM_INC_DIR}/tx_user.h COPYONLY)
    target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CUSTOM_INC_DIR}
    )

else()
    # A place for generated/copied include files (no need to change)
    set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

    # If the user provided an override, copy it to the custom directory
    if (NOT TX_USER_FILE)
        message(STATUS "Using default tx_user.h file")
        set(TX_USER_FILE ${AIC_CFG_PATH}/config/tx_user.h)
    else()
        message(STATUS "Using custom tx_user.h file from ${TX_USER_FILE}")
    endif()
    configure_file(${TX_USER_FILE} ${CUSTOM_INC_DIR}/tx_user.h COPYONLY)

    # Add the Common/inc directory to the project include list
    aic_include_directories(
        ${CUSTOM_INC_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/common/inc
    )
    target_include_directories(aic_interface
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN}/inc
        ${CMAKE_CURRENT_LIST_DIR}/utility/rtos_compatibility_layers/posix/inc
    )

endif()

aic_compile_definitions("TX_INCLUDE_USER_DEFINE_FILE" )

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
