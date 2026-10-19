# SPDX-License-Identifier: Apache-2.0

assert(CMAKE_SCRIPT_PATH "Toolchain root path invalid: please set the CMAKE_SCRIPT_PATH-variable")

# Do not set cached CONFIG_TOOLCHAIN_VARIANT, it set at Kconfig.
#set(CONFIG_TOOLCHAIN_VARIANT ${CONFIG_TOOLCHAIN_VARIANT} CACHE STRING "AIC toolchain variant")

# Configure the toolchain based on what SDK/toolchain is in use.
include(${CMAKE_SCRIPT_PATH}/cmake/toolchain/${CONFIG_TOOLCHAIN_VARIANT}/generic.cmake)

# Configure the toolchain based on what toolchain technology is used
# (gcc, host-gcc etc.)
include(${CMAKE_SCRIPT_PATH}/cmake/compiler/${COMPILER}/generic.cmake OPTIONAL)
include(${CMAKE_SCRIPT_PATH}/cmake/linker/${LINKER}/generic.cmake OPTIONAL)
include(${CMAKE_SCRIPT_PATH}/cmake/bintools/${BINTOOLS}/generic.cmake OPTIONAL)
