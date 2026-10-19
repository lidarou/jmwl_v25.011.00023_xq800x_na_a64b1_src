#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

# set compiler options
macro(toolchain_compiler_flags)
    add_compile_options(
        --specs=nano.specs
        -Wall
	-Werror
        -Wno-format
        -Wno-return-type
        -Wno-unused-but-set-variable
        -c
        -Os
        -fdata-sections
        -ffunction-sections
        -fno-builtin
        -fshort-enums
        -funsigned-char
        -mthumb
        -nostdlib
        -std=c99
        -msoft-float
    )
endmacro()

# set linker options
macro(toolchain_linker_flags)
    add_link_options(
        --entry=Reset_Handler
        --specs=nano.specs
        -nostdlib
        LINKER:-check-sections
        LINKER:-fatal-warnings
        LINKER:--gc-sections
        LINKER:--no-wchar-size-warning
        LINKER:--print-memory-usage
    )
endmacro()
