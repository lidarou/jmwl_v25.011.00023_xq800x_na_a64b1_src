# @Intent: Set compiler flags to enable buffer overflow checks in libc functions
# @config in CONFIG_NO_OPTIMIZATIONS optional : Optimizations may affect security
aic_compile_definitions($<TARGET_PROPERTY:compiler,security_fortify> )

# @Intent: Set compiler flags to detect general stack overflows across all functions
if(CONFIG_STACK_CANARIES)
  aic_compile_options($<TARGET_PROPERTY:compiler,security_canaries>)
endif()

if(BUILD_VERSION)
  aic_compile_definitions(
    BUILD_VERSION=${BUILD_VERSION}
  )
endif()
# @Intent: Obtain compiler optimizations flags and store in variables
# @details:
#   Kconfig.aic "Optimization level" is a kconfig choice, ensuring
#   only *one* of CONFIG_{NO,DEBUG,SPEED,SIZE}_OPTIMIZATIONS is set.
#   Refer to Kconfig.aic for selection logic and description of these choices.
#   toolchain_cc_optimize_*() macros must provide the mapping from these kconfigs
#   to compiler flags. Each macro will store the flags in a CMake variable, whose
#   name is passed as argument (somewhat like by reference).
#
#   If the user wants to tweak the optimizations, there are two ways:
#    1) Using EXTRA_CFLAGS which is applied regardless of kconfig choice, or
#    2) Rely on override support being implemented by your toolchain_cc_optimize_*()
#
get_property(OPTIMIZE_FOR_NO_OPTIMIZATIONS_FLAG TARGET compiler PROPERTY no_optimization)
get_property(OPTIMIZE_FOR_DEBUG_FLAG TARGET compiler PROPERTY optimization_debug)
get_property(OPTIMIZE_FOR_SPEED_FLAG TARGET compiler PROPERTY optimization_speed)
get_property(OPTIMIZE_FOR_SIZE_FLAG  TARGET compiler PROPERTY optimization_size)
get_property(OPTIMIZE_FOR_FAST_FLAG TARGET compiler PROPERTY optimization_fast)

# From kconfig choice, pick the actual OPTIMIZATION_FLAG to use.
# Kconfig choice ensures only one of these CONFIG_*_OPTIMIZATIONS is set.
if(CONFIG_NO_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_NO_OPTIMIZATIONS_FLAG})
elseif(CONFIG_DEBUG_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_DEBUG_FLAG})
elseif(CONFIG_SPEED_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_SPEED_FLAG})
elseif(CONFIG_FAST_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_FAST_FLAG})
elseif(CONFIG_SIZE_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_SIZE_FLAG}) # Default in kconfig
else()
  assert(0 "Unreachable code. Expected optimization level to have been chosen. See Kconfig.aic")
endif()

# Apply the final optimization flag(s)
aic_compile_options(${OPTIMIZATION_FLAG})

# @Intent: Obtain compiler specific flags related to C++ that are not influenced by kconfig
aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,required>>)

# @Intent: Obtain compiler specific flags for compiling under different ISO standards of C++
if(CONFIG_CPLUSPLUS)
  # From kconfig choice, pick a single dialect.
  # Kconfig choice ensures only one of these CONFIG_STD_CPP* is set.
  if(CONFIG_STD_CPP98)
    set(STD_CPP_DIALECT_FLAGS $<TARGET_PROPERTY:compiler-cpp,dialect_cpp98>)
  elseif(CONFIG_STD_CPP11)
    set(STD_CPP_DIALECT_FLAGS $<TARGET_PROPERTY:compiler-cpp,dialect_cpp11>) # Default in kconfig
  elseif(CONFIG_STD_CPP14)
    set(STD_CPP_DIALECT_FLAGS $<TARGET_PROPERTY:compiler-cpp,dialect_cpp14>)
  elseif(CONFIG_STD_CPP17)
    set(STD_CPP_DIALECT_FLAGS $<TARGET_PROPERTY:compiler-cpp,dialect_cpp17>)
  elseif(CONFIG_STD_CPP2A)
    set(STD_CPP_DIALECT_FLAGS $<TARGET_PROPERTY:compiler-cpp,dialect_cpp2a>)
  elseif(CONFIG_STD_CPP20)
    set(STD_CPP_DIALECT_FLAGS $<TARGET_PROPERTY:compiler-cpp,dialect_cpp20>)
  elseif(CONFIG_STD_CPP2B)
    set(STD_CPP_DIALECT_FLAGS $<TARGET_PROPERTY:compiler-cpp,dialect_cpp2b>)
  else()
    assert(0 "Unreachable code. Expected C++ standard to have been chosen. See Kconfig.aic.")
  endif()

  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:${STD_CPP_DIALECT_FLAGS}>)
endif()

if(NOT CONFIG_EXCEPTIONS)
  # @Intent: Obtain compiler specific flags related to C++ Exceptions
  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,no_exceptions>>)
endif()

if(NOT CONFIG_RTTI)
  # @Intent: Obtain compiler specific flags related to C++ Run Time Type Information
  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,no_rtti>>)
endif()

if(CONFIG_MISRA_SANE)
  # @Intent: Obtain toolchain compiler flags relating to MISRA.
  aic_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,warning_error_misra_sane>>)
  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,warning_error_misra_sane>>)
endif()

# This is intend to be temporary. Once we have fixed the violations that
# prevents build AIC, these flags shall be part of the default flags.
if(CONFIG_CODING_GUIDELINE_CHECK)
  # @Intent: Obtain toolchain compiler flags relating to coding guideline
  aic_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,warning_error_coding_guideline>>)
endif()

# @Intent: Set compiler specific macro inclusion of AUTOCONF_H
aic_compile_options("SHELL: $<TARGET_PROPERTY:compiler,imacros> ${AUTOCONF_H}")

if(NOT DEFINED CONFIG_COMPILER_AC5)
  # @Intent: Set compiler specific flag for bare metal freestanding option
  aic_compile_options($<TARGET_PROPERTY:compiler,freestanding>)

  # @Intent: Set compiler specific flag for tentative definitions, no-common
  aic_compile_options($<TARGET_PROPERTY:compiler,no_common>)

  # @Intent: Set compiler specific flag for production of debug information
  aic_compile_options($<TARGET_PROPERTY:compiler,debug>)
endif()

if(CONFIG_COMPILER_COLOR_DIAGNOSTICS)
  # @Intent: Set compiler specific flag for diagnostic messages
  aic_compile_options($<TARGET_PROPERTY:compiler,diagnostic>)
endif()

aic_compile_options(
  ${TOOLCHAIN_C_FLAGS}
)

# @Intent: Obtain compiler specific flags related to assembly
# ToDo: Remember to get feedback from Oticon on this, as they might use the `ASM_BASE_FLAG` since this is done this way.
aic_compile_options($<$<COMPILE_LANGUAGE:ASM>:$<TARGET_PROPERTY:asm,required>>)

# Common toolchain-agnostic assembly flags
aic_compile_options(
  $<$<COMPILE_LANGUAGE:ASM>:-D_ASMLANGUAGE>
)

# @Intent: Set fundamental linker specific flags
toolchain_ld_base()

if (NOT "${COMPILER}" STREQUAL "clang")
    aic_ld_options("-ereset_handler")
    aic_ld_options("-Wl,--print-memory-usage")

    if(NOT CONFIG_NATIVE_APPLICATION)
      # @Intent: Set linker specific flags for bare metal target
      toolchain_ld_baremetal()
    endif()

    if(CONFIG_LIB_CPLUSPLUS)
      # @Intent: Set linker specific flags for C++
      toolchain_ld_cpp()
    endif()

    aic_cc_option(--specs=nano.specs)
    if(DEFINED CONFIG_LINKER_PRINT_FLOAT)
        aic_ld_options(-u_printf_float)
        # aic_ld_options(-u_scanf_float)
    endif()
endif()

if(NOT DEFINED CONFIG_COMPILER_AC5)
  # @Intent: Add the basic toolchain warning flags
  aic_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,warning_base>>)
  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,warning_base>>)
endif()

# ==========================================================================
#
# cmake -DW=... settings
#
# W=1 - warnings that may be relevant and does not occur too often
# W=2 - warnings that occur quite often but may still be relevant
# W=3 - the more obscure warnings, can most likely be ignored
# ==========================================================================
# @Intent: Add cmake -DW toolchain supported warnings, if any
if(W MATCHES "1")
  aic_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,warning_dw_1>>)
  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,warning_dw_1>>)
endif()

if(W MATCHES "2")
  aic_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,warning_dw_2>>)
  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,warning_dw_2>>)
endif()

if(W MATCHES "3")
  aic_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,warning_dw_3>>)
  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,warning_dw_3>>)
endif()

if(NOT DEFINED CONFIG_COMPILER_AC5)
  # @Intent: Add extended, more specific, toolchain warning flags
  aic_compile_options($<TARGET_PROPERTY:compiler,warning_extended>)

  # @Intent: Trigger an error when a declaration does not specify a type
  aic_compile_options($<$<COMPILE_LANGUAGE:C>:$<TARGET_PROPERTY:compiler,warning_error_implicit_int>>)
  aic_compile_options($<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:compiler-cpp,warning_error_implicit_int>>)

  # Put functions and data in their own binary sections so that ld can
  # garbage collect them
  aic_cc_option(-ffunction-sections -fdata-sections)

  aic_cc_option(-fno-pie)
  aic_cc_option(-fno-pic)
  aic_cc_option(-fno-builtin)
else()
  aic_cc_option(--c99)
  aic_cc_option(-g)
  aic_cc_option(--inline)
  aic_cc_option(--split_sections)
  aic_cc_option(--preprocess_assembly)
  aic_cc_option(-c)
  aic_cc_option(--apcs=interwork)
endif()

if(CONFIG_THREAD_LOCAL_STORAGE)
# Only support local exec TLS model at this point.
aic_cc_option(-ftls-model=local-exec)
endif()

if(CONFIG_OVERRIDE_FRAME_POINTER_DEFAULT)
  if(CONFIG_OMIT_FRAME_POINTER)
    aic_cc_option(-fomit-frame-pointer)
  else()
    aic_cc_option(-fno-omit-frame-pointer)
  endif()
endif()

separate_arguments(COMPILER_OPT_AS_LIST UNIX_COMMAND ${CONFIG_COMPILER_OPT})
aic_compile_options(${COMPILER_OPT_AS_LIST})
