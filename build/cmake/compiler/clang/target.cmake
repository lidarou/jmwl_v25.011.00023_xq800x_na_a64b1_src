# SPDX-License-Identifier: Apache-2.0

# Configuration for host installed clang
#

set(NOSTDINC "")

# Note that NOSYSDEF_CFLAG may be an empty string, and
# set_ifndef() does not work with empty string.
if(NOT DEFINED NOSYSDEF_CFLAG)
  set(NOSYSDEF_CFLAG -undef)
endif()

if(DEFINED TOOLCHAIN_HOME)
  set(find_program_clang_args PATHS ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
endif()

if(DEFINED CONFIG_COMPILER_AC5)
  find_program(CMAKE_C_COMPILER   armcc     ${find_program_clang_args})
  find_program(CMAKE_CXX_COMPILER armcc     ${find_program_clang_args})
else()
  find_program(CMAKE_C_COMPILER   armclang  ${find_program_clang_args})
  find_program(CMAKE_CXX_COMPILER armclang  ${find_program_clang_args})
endif()
find_program(CMAKE_ELFDUMP        fromelf   ${find_program_clang_args})
find_program(CMAKE_LINKER         armlink   ${find_program_clang_args})

if(NOT "${ARCH}" STREQUAL "posix")
  include(${CMAKE_SCRIPT_PATH}/cmake/gcc-m-cpu.cmake)

  if("${ARCH}" STREQUAL "arm")
    if(DEFINED CONFIG_COMPILER_AC6)
      list(APPEND TOOLCHAIN_C_FLAGS -fshort-enums)
    endif()
    # ac5/6 need not this link option
    #list(APPEND TOOLCHAIN_LD_FLAGS -fshort-enums)

    include(${CMAKE_SCRIPT_PATH}/cmake/compiler/clang/${CONFIG_COMPILER_NAME}/target_arm.cmake)
  endif()

endif()

# Load toolchain_cc-family macros

macro(toolchain_cc_nostdinc)
  if(NOT "${ARCH}" STREQUAL "posix")
    aic_compile_options( -nostdinc)
  endif()
endmacro()
