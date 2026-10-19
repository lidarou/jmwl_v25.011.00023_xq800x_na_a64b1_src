# SPDX-License-Identifier: Apache-2.0

find_program(CMAKE_LINKER     ld.lld )

set_ifndef(LINKERFLAGPREFIX -Wl)
set_ifndef(TOPT --scatter)

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
    message(WARNING "This generator is not well supported. The
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
    ${extra_dependencies}
    # NB: 'linker_script_dep' will use a keyword that ends 'DEPENDS'
    ${linker_script_dep}
    COMMAND ${CMAKE_C_COMPILER}
    -x assembler-with-cpp
    ${NOSYSDEF_CFLAG}
    -MD -MF ${linker_script_gen}.dep -MT ${base_name}/${linker_script_gen}
    -D_LINKER
    -D_ASMLANGUAGE
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

  target_link_libraries(
    ${TOOLCHAIN_LD_LINK_ELF_TARGET_ELF}
    ${TOOLCHAIN_LD_LINK_ELF_LIBRARIES_PRE_SCRIPT}
    ${TOPT}
    ${TOOLCHAIN_LD_LINK_ELF_LINKER_SCRIPT}
    ${TOOLCHAIN_LD_LINK_ELF_LIBRARIES_POST_SCRIPT}

    --list=${TOOLCHAIN_LD_LINK_ELF_OUTPUT_MAP}
    ${AIC_LIBS_PROPERTY}
    ${AIC_EXP_LIBS_PROPERTY}
    # all the external libs add first
    ${AIC_EXT_LIBS_PROPERTY}
    #$<TARGET_OBJECTS:${OFFSETS_LIB}>
    #${LIB_INCLUDE_DIR}
    #-L${AIC_LIBS_DIR}
    #${TOOLCHAIN_LIBS}

    ${TOOLCHAIN_LD_LINK_ELF_DEPENDENCIES}
  )

  #
  # Why add link cmd again!!!???
  # We use static lib for every root directory in our aicos system,
  # and at the root dir there has nothing source code, the elf is linked
  # by all the static libraries.
  # But the init* section will be optimized by armclang by default cmd,
  # it needs to add "(*.o)" postfix at the static lib name, and needs to
  # enclose the static lib name, but ninja and default cmake api do not
  # support.
  #
  get_property(AIC_LD_PARAMS GLOBAL PROPERTY AIC_LD_FLAGS)
  add_prefix(PREFIX_LIBS_NAME "lib/lib" ${AIC_LIBS_PROPERTY})
  add_postfix(AIC_LINK_LIBS_NAME ".a (*.o)" ${PREFIX_LIBS_NAME})

  add_prefix(PREFIX_EXP_LIBS_NAME "lib/lib" ${AIC_EXP_LIBS_PROPERTY})
  add_postfix(AIC_LINK_EXP_LIBS_NAME ".a" ${PREFIX_EXP_LIBS_NAME})

  add_prefix(PREFIX_EXT_LIBS_NAME "lib/lib" ${AIC_EXT_LIBS_PROPERTY})
  add_postfix(AIC_LINK_EXT_LIBS_NAME ".a" ${PREFIX_EXT_LIBS_NAME})

  add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_LINKER} ${TOPT} ${TOOLCHAIN_LD_LINK_ELF_LINKER_SCRIPT}
    --list=${TOOLCHAIN_LD_LINK_ELF_OUTPUT_MAP}
    ${AIC_LINK_LIBS_NAME}
    ${AIC_LINK_EXP_LIBS_NAME}
    ${AIC_LINK_EXT_LIBS_NAME}
    ${AIC_LD_PARAMS}
    "-o"
    ${TOOLCHAIN_LD_LINK_ELF_TARGET_ELF}.elf
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
    COMMAND ${CMAKE_ELFDUMP} --bin -o ${output_bin} ${input_elf}
  )
endfunction(toolchain_objcopy_binary)

# Load toolchain_ld-family macros
include(${CMAKE_SCRIPT_PATH}/cmake/linker/lld/target_base.cmake)
#ac6 should not include these linker config, because them can't be recognized.
#include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_base.cmake)
#include(${CMAKE_SCRIPT_PATH}/cmake/linker/${LINKER}/target_baremetal.cmake)
#include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_cpp.cmake)
#include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_relocation.cmake)
#include(${CMAKE_SCRIPT_PATH}/cmake/linker/ld/target_configure.cmake)
