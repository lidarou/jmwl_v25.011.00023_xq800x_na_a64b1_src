#
# Set customized cmake path for aic, both of these path could be used
#
set(AIC_SOURCE_ROOT ${CMAKE_SOURCE_DIR})
set(AIC_ORIGIN_ROOT ${CMAKE_SOURCE_DIR}/..)
set(AIC_BUILD_ROOT  ${CMAKE_BINARY_DIR})
set(AIC_LIBS_DIR    "${AIC_BUILD_ROOT}/lib")
set(AIC_HWI_DIR     "${CMAKE_SOURCE_DIR}/../hwi/${CMAKE_CONFIG_CHIP}/${CMAKE_SUBSYS_CONFIG}")
set(AIC_PINLST_DIR "${CMAKE_SOURCE_DIR}/../hwi/${CMAKE_CONFIG_CHIP}")
set(AIC_PPINLST_DIR  "${CMAKE_SOURCE_DIR}/../hwi")
set(AIC_CHIP_COM_DIR "${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/common_inc")

#
# Setup basic function and macro
#
include(${CMAKE_CURRENT_LIST_DIR}/utilities.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/extensions.cmake)

#
# Setup python interparser
#
include(${CMAKE_CURRENT_LIST_DIR}/python.cmake)

#
# Configure projects default config
#

# Used by kconfig.cmake
set(KCONFIG_ROOT ${CMAKE_SOURCE_DIR}/Kconfig)

# Re-configure (Re-execute all CMakeLists.txt code) when autoconf.h changes
set(AUTOCONF_H ${AIC_BUILD_ROOT}/auto_generate/kconfig/include/generated/autoconf.h)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${AUTOCONF_H})

#
# Perform Kconfig to update CONFIG_* Variable
# NOTE:Only after this , you can use CONFIG_* varible from Kconfig
#
include(${CMAKE_CURRENT_LIST_DIR}/kconfig.cmake)

#
# Set AIC board configuration path
#
set(AIC_SUBSYS_PATH     "${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/${CMAKE_SUBSYS_CONFIG}")
set(AIC_CFG_PATH        "${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/${CMAKE_SUBSYS_CONFIG}/${CMAKE_CONFIG_BOARD}")
set(AIC_TARGET_DIR      "${CMAKE_SOURCE_DIR}/../targets/${CMAKE_CONFIG_CHIP}/${CMAKE_CONFIG_PROJECT}")
set(AIC_OUT_DIR         "${CMAKE_SOURCE_DIR}/../out/${CMAKE_CONFIG_PROJECT}/${CMAKE_SUBSYS_CONFIG}")
set(AIC_CHIP_DIR        "${AIC_OUT_DIR}/include")
set(AIC_PMIC_DIR        "${CMAKE_SOURCE_DIR}/../hwi/${CONFIG_PMIC_VERSION}")
set(AIC_SDK_LIBS_DIR    "${AIC_ORIGIN_ROOT}/libs/${CMAKE_SDK_BASE_BOARD}/${CMAKE_SUBSYS_CONFIG}")
set(AIC_NVRAM_COM_DIR   "${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/nvram/${CMAKE_SUBSYS_CONFIG}/common")
set(AIC_NVRAM_CFG_PATH  "${CMAKE_SOURCE_DIR}/projects/${CMAKE_CONFIG_CHIP}/nvram/${CMAKE_SUBSYS_CONFIG}/${CMAKE_NVRAM_BOARD}")

#
# "aic_interface" is a source-less library that encapsulates all the global
# compiler options needed by all source files. All aic libraries,
# including the library named "aic" link with this library to
# obtain these flags.
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#interface-libraries
#
add_library(aic_interface INTERFACE)

# this makes the test compiles use static library option so that we don't need to pre-set linker flags and scripts
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# generate compile commands json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#
# Setup Cross-Compile Tools and basic copile flags
#
include(${CMAKE_SCRIPT_PATH}/cmake/generic_toolchain.cmake)
include(${CMAKE_SCRIPT_PATH}/cmake/target_toolchain.cmake)
