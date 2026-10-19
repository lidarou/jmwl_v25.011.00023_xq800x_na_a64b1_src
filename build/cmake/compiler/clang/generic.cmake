# SPDX-License-Identifier: Apache-2.0

if(DEFINED TOOLCHAIN_HOME)
  set(find_program_clang_args PATHS ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
endif()

if(DEFINED CONFIG_COMPILER_AC5)
  find_program(CMAKE_C_COMPILER armcc ${find_program_clang_args})
else()
  find_program(CMAKE_C_COMPILER armclang ${find_program_clang_args})
endif()
