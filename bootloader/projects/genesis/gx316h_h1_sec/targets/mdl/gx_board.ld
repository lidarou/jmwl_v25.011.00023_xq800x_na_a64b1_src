#/****************************************************************************/
#/*                                                                          */
#/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
#/*                                                                          */
#/****************************************************************************/

/* Linker script to configure memory regions. */
/* This file will be run trough the pre-processor. */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x20004000, LENGTH = 0x9000
    RAM   (rwx) : ORIGIN = 0x20032000, LENGTH = 0x12000
}

__heap_size__  = 0x8000;
__msp_stack_size__ = 0x1000;

GROUP(libgcc.a libc.a libm.a libnosys.a)
ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))
        __Vectors_End = .;
        __Vectors_Size = __Vectors_End - __Vectors;


        *(.text .text.* .glue_7t .glue_7 .gnu.linkonce.t.* .gcc_except_table .gnu.sgstubs)
        . = ALIGN(4);
        __initcall_begin = .;
        KEEP(*(.initcall1.init))
        KEEP(*(.initcall2.init))
        KEEP(*(.initcall3.init))
        KEEP(*(.initcall4.init))
        KEEP(*(.initcall5.init))
        KEEP(*(.initcall6.init))
        KEEP(*(.initcall7.init))
        KEEP(*(.initcall8.init))
        __initcall_end = .;

	. = ALIGN(4);
        *(.rodata*)
    } > FLASH

    .dtors :
    {
        __dtors_start__ = .;
        KEEP (*(SORT(.dtors.*))) KEEP (*(.dtors))
        __dtors_end__ = .;
    } > FLASH

    .ctors :
    {
        __ctors_start__ = .;
        KEEP (*(SORT(.ctors.*))) KEEP (*(.ctors))
        __ctors_end__ = .;
    } > FLASH

    .rodata :
    {
        __rodata_start__ = .;
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        KEEP (*(SORT(.auto_init*)))
        KEEP (*(SORT(.aiccmd*)))
        __rodata_end__ = .;
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    __data_load_start__ = __exidx_end;
    .data : ALIGN(4)
    {
       __data_start__ = .;
        *(.data*)
       __data_end__ = .;
    } > RAM AT > FLASH

    .bss : ALIGN(4)
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM AT > RAM

    .msp_stack : ALIGN(32)
    {
        __stack_start__ = .;
		*(.msp_stack)
        . += __msp_stack_size__;
        __stack_end__ = ALIGN(8);
    } > RAM

    .heap : ALIGN(8)
    {
        __heap_start__ = .;
        PROVIDE(end = .);
        . += __heap_size__;
        __heap_end__ = .;
    } > RAM
}
