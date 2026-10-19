# SPDX-License-Identifier: Apache-2.0

if(DEFINED TOOLCHAIN_HOME)
  # When Toolchain home is defined, then we are cross-compiling, so only look
  # for linker in that path, else we are using host tools.
  set(LD_SEARCH_PATH PATHS ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
endif()

find_program(CMAKE_LINKER ${CROSS_COMPILE}ld.bfd ${LD_SEARCH_PATH})
if(NOT CMAKE_LINKER)
  find_program(CMAKE_LINKER ${CROSS_COMPILE}ld ${LD_SEARCH_PATH})
endif()

set_ifndef(LINKERFLAGPREFIX -Wl)

if(NOT "${CONFIG_TOOLCHAIN_VARIANT}" STREQUAL "host")
  if(CONFIG_EXCEPTIONS)
    # When building with C++ Exceptions, it is important that crtbegin and crtend
    # are linked at specific locations.
    # The location is so important that we cannot let this be controlled by normal
    # link libraries, instead we must control the link command specifically as
    # part of toolchain.
    set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${LIBGCC_DIR}/crtbegin.o <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${LIBGCC_DIR}/crtend.o")
  endif()
endif()

# Run $LINKER_SCRIPT file through the C preprocessor, producing ${linker_script_gen}
# NOTE: ${linker_script_gen} will be produced at build-time; not at configure-time
macro(configure_linker_script linker_script_gen linker_pass_define)
  set(extra_dependencies ${ARGN})

  # Different generators deal with depfiles differently.
  if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    # Note that the IMPLICIT_DEPENDS option is currently supported only
    # for Makefile generators and will be ignored by other generators.
    set(linker_script_dep IMPLICIT_DEPENDS C ${LINKER_SCRIPT})
  elseif(CMAKE_GENERATOR STREQUAL "Ninja")
    # Using DEPFILE with other generators than Ninja is an error.
    set(linker_script_dep DEPFILE ${PROJECT_BINARY_DIR}/${linker_script_gen}.dep)
  else()
    # TODO: How would the linker script dependencies work for non-linker
    # script generators.
    message(STATUS "Warning; this generator is not well supported. The
  Linker script may not be regenerated when it should.")
    set(linker_script_dep "")
  endif()

  aic_get_include_directories_for_lang(C current_includes)
  get_filename_component(base_name ${CMAKE_CURRENT_BINARY_DIR} NAME)
  get_property(current_defines GLOBAL PROPERTY PROPERTY_LINKER_SCRIPT_DEFINES)

  add_custom_command(
    OUTPUT ${linker_script_gen}
    DEPENDS
    ${LINKER_SCRIPT}
    ${AUTOCONF_H}
    ${extra_dependencies}
    # NB: 'linker_script_dep' will use a keyword that ends 'DEPENDS'
    ${linker_script_dep}
    COMMAND ${CMAKE_C_COMPILER}
    -x assembler-with-cpp
    ${NOSYSDEF_CFLAG}
    -MD -MF ${linker_script_gen}.dep -MT ${base_name}/${linker_script_gen}
    -D_LINKER
    -D_ASMLANGUAGE
    -imacros ${AUTOCONF_H}
    ${current_includes}
    ${current_defines}
    ${linker_pass_define}
    -E ${LINKER_SCRIPT}
    -P # Prevent generation of debug `#line' directives.
    -o ${linker_script_gen}
    VERBATIM
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    COMMAND_EXPAND_LISTS
  )
endmacro()

# Force symbols to be entered in the output file as undefined symbols
function(toolchain_ld_force_undefined_symbols)
  foreach(symbol ${ARGN})
    aic_link_libraries(${LINKERFLAGPREFIX},-u,${symbol})
  endforeach()
endfunction()

# Link a target to given libraries with toolchain-specific argument order
#
# Usage:
#   toolchain_ld_link_elf(
#     TARGET_ELF             <target_elf>
#     OUTPUT_MAP             <output_map_file_of_target>
#     LIBRARIES_PRE_SCRIPT   [libraries_pre_script]
#     LINKER_SCRIPT          <linker_script>
#     LIBRARIES_POST_SCRIPT  [libraries_post_script]
#     DEPENDENCIES           [dependencies]
#   )
function(toolchain_ld_link_elf)
  cmake_parse_arguments(
    TOOLCHAIN_LD_LINK_ELF                                     # prefix of output variables
    ""                                                        # list of names of the boolean arguments
    "TARGET_ELF;OUTPUT_MAP;LINKER_SCRIPT"                     # list of names of scalar arguments
    "LIBRARIES_PRE_SCRIPT;LIBRARIES_POST_SCRIPT;DEPENDENCIES" # list of names of list arguments
    ${ARGN}                                                   # input args to parse
  )

  if(${CMAKE_LINKER} STREQUAL "${CROSS_COMPILE}ld.bfd")
    # ld.bfd was found so let's explicitly use that for linking, see #32237
    set(use_linker "-fuse-ld=bfd")
  endif()

  # Remove libaraies to remake determintic static library
  # Make source sdk make all is clean relative to make all
  # the "execute_process" api will be executed immediately when
  # call this cmake api, but target_link_libraries will be
  # execute at the last of project build for link.
  if(CONFIG_SDK_ENABLE OR (CONFIG_SDK_ENABLE AND CONFIG_CP_OPEN_SOURCE))
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/lib
    )
    if (EXISTS ${CMAKE_BINARY_DIR}/libs-sdk)
        execute_process(
          COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/libs-sdk
            ${AIC_ORIGIN_ROOT}/libs/${CMAKE_SDK_BASE_BOARD}/${CMAKE_SUBSYS_CONFIG}
        )
    endif()
  else()
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/lib
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/libs-sdk
    )
  endif()

  if(CONFIG_CONSUMER_SDK)
    if (EXISTS ${CMAKE_BINARY_DIR}/libs-sdk-c)
        execute_process (
          COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/libs-sdk-c
            ${AIC_ORIGIN_ROOT}/libs/${CMAKE_SDK_BASE_BOARD}/${CMAKE_SUBSYS_CONFIG}
        )
    endif()
  endif()

  # Include all the sdk libraries into AIC_SDK_LIBS_DIR,
  # and all the sdk compile will add all these libraries directly.
  if (CONFIG_SDK_ENABLE AND EXISTS ${AIC_SDK_LIBS_DIR})
    file(GLOB AIC_SDK_LIBS_PROPERTY ${AIC_SDK_LIBS_DIR}/*.a)
  endif()

  target_link_libraries(
    ${TOOLCHAIN_LD_LINK_ELF_TARGET_ELF}
    ${TOOLCHAIN_LD_LINK_ELF_LIBRARIES_PRE_SCRIPT}
    ${use_linker}
    ${TOPT}
    ${TOOLCHAIN_LD_LINK_ELF_LINKER_SCRIPT}
    ${TOOLCHAIN_LD_LINK_ELF_LIBRARIES_POST_SCRIPT}

    ${LINKERFLAGPREFIX},-Map=${TOOLCHAIN_LD_LINK_ELF_OUTPUT_MAP}
    ${LINKERFLAGPREFIX},--whole-archive
    ${AIC_LIBS_PROPERTY}
    ${AIC_SDK_LIBS_PROPERTY}
    ${AIC_EXP_LIBS_PROPERTY}
    ${LINKERFLAGPREFIX},--no-whole-archive
    # all the external libs add first
    ${AIC_EXT_LIBS_PROPERTY}
    #$<TARGET_OBJECTS:${OFFSETS_LIB}>
    ${LIB_INCLUDE_DIR}
    -L${AIC_LIBS_DIR}
    ${TOOLCHAIN_LIBS}

    ${TOOLCHAIN_LD_LINK_ELF_DEPENDENCIES}
  )
endfunction(toolchain_ld_link_elf)

# Objcopy from elf to binary file
#
# Usage:
#   toolchain_objcopy_binary(
#     INPUT_ELF
#     OUTPUT_BIN
#   )
function(toolchain_objcopy_binary input_elf output_bin)
  add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${OBJCOPY} -O binary ${input_elf} ${output_bin}
  )
endfunction(toolchain_objcopy_binary)

# Objcopy from elf to multi binary file
#
# Usage:
#   toolchain_objcopy_multi_binary(
#     INPUT_ELF
#     OUTPUT_BIN
#   )
function(toolchain_objcopy_multi_binary input_elf outpu_bin sections)
  add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${OBJCOPY} -O binary ${input_elf} ${outpu_bin} -j .${sections}*
  )
endfunction(toolchain_objcopy_multi_binary)

# Load toolchain_ld-family macros
include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_base.cmake)
include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_baremetal.cmake)
include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_cpp.cmake)
include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_relocation.cmake)
include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_configure.cmake)
