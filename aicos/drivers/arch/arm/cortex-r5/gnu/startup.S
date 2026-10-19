@/**************************************************************************/
@/*                                                                        */
@/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
@/*                                                                        */
@/*       This software is licensed under the Microsoft Software License   */
@/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
@/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
@/*       and in the root directory of this software.                      */
@/*                                                                        */
@/**************************************************************************/
@
@
@/**************************************************************************/
@/**************************************************************************/
@/**                                                                       */ 
@/** ThreadX Component                                                     */ 
@/**                                                                       */
@/**   Initialize                                                          */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/
@
@
@#define TX_SOURCE_CODE
@
@
@/* Include necessary system files.  */
@
@#include "tx_api.h"
@#include "tx_initialize.h"
@#include "tx_thread.h"
@#include "tx_timer.h"

    .arm

    .global      STARTUP
    .global      _start
    .global      reset_handler
#ifdef CONFIG_RTOS_THREADX
    .global      __tx_undefined
    .global      __tx_swi_interrupt
    .global      __tx_prefetch_handler
    .global      __tx_abort_handler
    .global      __tx_reserved_handler
    .global      __tx_irq_handler
    .global      __tx_fiq_handler
#elif defined(CONFIG_RTOS_LITEOS_M)
    .global      HalExceptFiqHdl
    .global      HalExceptResvHdl
    .global      HalExceptDataAbortHdl
    .global      HalExceptPrefetchAbortHdl
    .global      HalExceptSwiHdl
    .global      HalExceptUndefInstrHdl
    .global      HalExceptIrqHdl
#endif

@
@
@/* Define the vector area.  This should be located or copied to 0.  */
@
    .text
    .section .vectors,"ax"
    .global __vectors
__vectors:

    LDR     pc, STARTUP                     @ Reset goes to startup function
    LDR     pc, UNDEFINED                   @ Undefined handler
    LDR     pc, SWI                         @ Software interrupt handler
    LDR     pc, PREFETCH                    @ Prefetch exception handler
    LDR     pc, ABORT                       @ Abort exception handler
    LDR     pc, RESERVED                    @ Reserved exception handler
    LDR     pc, IRQ                         @ IRQ interrupt handler
#ifndef  CONFIG_DBG_CPPC_SAMP
    LDR     pc, FIQ                         @ FIQ interrupt handler
#else
@ fiq handle, save r14 to wt ptr and clear irq
@ r8 is pc_samp_base, r9 is pc_samp_end, r10 is tmr clear reg r11 is wt ptr

    sub  lr, lr, #4
    str lr, [r11]
    add r11, r11, #4
    cmp r11, r9
    it  eq
    moveq r11, r8

@ clr irq
    movs r12,#0x1
    str  r12,[r10]

@ go back
    movs pc, lr
#endif

STARTUP:
    .word  _start                           @ Reset goes to C startup function
#ifdef CONFIG_RTOS_THREADX
UNDEFINED:
    .word  __tx_undefined                   @ Undefined handler
SWI:
    .word  __tx_swi_interrupt               @ Software interrupt handler
PREFETCH:
    .word  __tx_prefetch_handler            @ Prefetch exception handler
ABORT:                             
    .word  __tx_abort_handler               @ Abort exception handler
RESERVED:                            
    .word  __tx_reserved_handler            @ Reserved exception handler
IRQ:                                  
    .word  __tx_irq_handler                 @ IRQ interrupt handler
FIQ:
    .word  __tx_fiq_handler                 @ FIQ interrupt handler
#elif defined(CONFIG_RTOS_LITEOS_M)
UNDEFINED:
    .word  HalExceptUndefInstrHdl           @ Undefined handler
SWI:
    .word  HalExceptSwiHdl                  @ Software interrupt handler
PREFETCH:
    .word  HalExceptPrefetchAbortHdl        @ Prefetch exception handler
ABORT:                             
    .word  HalExceptDataAbortHdl            @ Abort exception handler
RESERVED:                            
    .word  HalExceptResvHdl                 @ Reserved exception handler
IRQ:                                  
    .word  HalExceptIrqHdl                  @ IRQ interrupt handler
FIQ:
    .word  HalExceptFiqHdl                  @ FIQ interrupt handler
#endif
