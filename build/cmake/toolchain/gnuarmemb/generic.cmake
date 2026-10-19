# SPDX-License-Identifier: Apache-2.0

# Until we completely deprecate it
if(NOT DEFINED ENV{ARM_TOOLCHAIN_PATH})
    if(DEFINED ENV{GCCARMEMB_TOOLCHAIN_PATH})
        message(WARNING "GCCARMEMB_TOOLCHAIN_PATH is deprecated, please use ARM_TOOLCHAIN_PATH instead")
        set(ARM_TOOLCHAIN_PATH $ENV{GCCARMEMB_TOOLCHAIN_PATH})
    endif()
endif()

set_ifndef(ARM_TOOLCHAIN_PATH "$ENV{ARM_TOOLCHAIN_PATH}")
set(       ARM_TOOLCHAIN_PATH ${ARM_TOOLCHAIN_PATH} CACHE PATH "gnuarmemb install directory")
assert(    ARM_TOOLCHAIN_PATH "ARM_TOOLCHAIN_PATH is not set")

if(NOT EXISTS ${ARM_TOOLCHAIN_PATH})
    message(FATAL_ERROR "Nothing found at ARM_TOOLCHAIN_PATH: '${ARM_TOOLCHAIN_PATH}'")
endif()

set(TOOLCHAIN_HOME ${ARM_TOOLCHAIN_PATH}/..)

set(COMPILER gcc)
set(LINKER ld)
set(BINTOOLS gnu)

set(CROSS_COMPILE_TARGET arm-none-eabi)
set(SYSROOT_TARGET       arm-none-eabi)

set(CROSS_COMPILE ${TOOLCHAIN_HOME}/bin/${CROSS_COMPILE_TARGET}-)
set(SYSROOT_DIR   ${TOOLCHAIN_HOME}/${SYSROOT_TARGET})
set(TOOLCHAIN_HAS_NEWLIB ON CACHE BOOL "True if toolchain supports newlib")

message(STATUS "Found toolchain: gnuarmemb (${TOOLCHAIN_HOME})")
