# No special flags are needed for xcc.
# Only select whether gcc or clang flags should be inherited.
if(CC STREQUAL "clang")
  include(${CMAKE_SCRIPT_PATH}/cmake/compiler/clang/compiler_flags.cmake)
else()
  include(${CMAKE_SCRIPT_PATH}/cmake/compiler/gcc/compiler_flags.cmake)
endif()
