# SPDX-License-Identifier: Apache-2.0

# See root CMakeLists.txt for description and expectations of these macros

macro(toolchain_ld_base)

  # TOOLCHAIN_LD_FLAGS comes from compiler/gcc/target.cmake
  # LINKERFLAGPREFIX comes from linker/ld/target.cmake
  aic_ld_options(
    ${TOOLCHAIN_LD_FLAGS}
  )

  aic_ld_options(
    "--keep=__init_*"
    "--keep=cons_cmd_*"
    "--summary_stderr"
    "--info=summarysizes"
    "--map"
    "--xref"
    "--symbols"
    "--info=sizes"
    "--info=totals"
    "--info=unused"
    "--info=veneers"
  )

endmacro()
