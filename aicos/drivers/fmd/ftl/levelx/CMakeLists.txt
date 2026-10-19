cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(levelx
    VERSION 6.0.0 
    LANGUAGES C ASM
)

option(LX_STANDALONE_ENABLE "Enable LevelX in standalone mode" OFF)

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

aic_include_directories(common/inc)

# Define our target library and an alias for consumers
# aic_sdk_library is register close source code library
aic_sdk_library(${PROJECT_NAME})
if(NOT CONFIG_SDK_ENABLE)

    add_library("azrtos::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

    # Define any required dependencies between this library and others
    if(NOT LX_STANDALONE_ENABLE)
        target_link_libraries(${PROJECT_NAME} PUBLIC 
            "azrtos::threadx")
    endif()

    # A place for generated/copied include files (no need to change)
    set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

    # Include Levelx configurations for compiler
    # aic_compile_options("SHELL: $<TARGET_PROPERTY:compiler,imacros> ${AIC_CFG_PATH}/config/levelx_config.h")

    # Pick up the common stuff
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/common)

    # Include the user's override file if required
    if (NOT LX_USER_FILE)
        message(STATUS "Using default lx_user.h file")
        set(LX_USER_FILE ${CMAKE_CURRENT_LIST_DIR}/common/inc/lx_user_sample.h)
    else()
        message(STATUS "Using custom lx_user.h file from ${LX_USER_FILE}")
    endif()
    configure_file(${LX_USER_FILE} ${CUSTOM_INC_DIR}/lx_user.h COPYONLY)
    target_include_directories(${PROJECT_NAME} 
        PUBLIC 
        ${CUSTOM_INC_DIR}
    )

    if(NOT LX_STANDALONE_ENABLE)
        target_compile_definitions(${PROJECT_NAME} PUBLIC "LX_INCLUDE_USER_DEFINE_FILE" )
    else()
        # Enable LevelX and FileX standalone support (No Azure RTOS support)
        set(FX_STANDALONE_ENABLE ON CACHE BOOL "Standalone enable")
        target_compile_definitions(${PROJECT_NAME} PUBLIC "LX_INCLUDE_USER_DEFINE_FILE" -DLX_STANDALONE_ENABLE -DFX_STANDALONE_ENABLE)
    endif()

else()

    # A place for generated/copied include files (no need to change)
    set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

    # Include Levelx configurations for compiler
    # aic_compile_options("SHELL: $<TARGET_PROPERTY:compiler,imacros> ${AIC_CFG_PATH}/config/levelx_config.h")

    # Include the user's override file if required
    if (NOT LX_USER_FILE)
        message(STATUS "Using default lx_user.h file")
        set(LX_USER_FILE ${CMAKE_CURRENT_LIST_DIR}/common/inc/lx_user_sample.h)
    else()
        message(STATUS "Using custom lx_user.h file from ${LX_USER_FILE}")
    endif()
    configure_file(${LX_USER_FILE} ${CUSTOM_INC_DIR}/lx_user.h COPYONLY)
    aic_include_directories(
        ${CUSTOM_INC_DIR}
    )

    if(NOT LX_STANDALONE_ENABLE)
        aic_compile_definitions("LX_INCLUDE_USER_DEFINE_FILE")
    else()
        # Enable LevelX and FileX standalone support (No Azure RTOS support)
        set(FX_STANDALONE_ENABLE ON CACHE BOOL "Standalone enable")
        aic_compile_definitions("LX_INCLUDE_USER_DEFINE_FILE" -DLX_STANDALONE_ENABLE -DFX_STANDALONE_ENABLE)
    endif()

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
