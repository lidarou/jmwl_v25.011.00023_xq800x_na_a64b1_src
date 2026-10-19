;/**************************************************************************/
;/*                                                                        */
;/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
;/*                                                                        */
;/*       This software is licensed under the Microsoft Software License   */
;/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
;/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
;/*       and in the root directory of this software.                      */
;/*                                                                        */
;/**************************************************************************/
;
;
;/**************************************************************************/
;/**************************************************************************/
;/**                                                                       */
;/** ThreadX Component                                                     */
;/**                                                                       */
;/**   Initialize                                                          */
;/**                                                                       */
;/**************************************************************************/
;/**************************************************************************/
;
;
;#define TX_SOURCE_CODE
;
;
;/* Include necessary system files.  */
;
;#include "tx_api.h"
;#include "tx_initialize.h"
;#include "tx_thread.h"
;#include "tx_timer.h"

    ;.eabi_attribute Tag_ABI_align_preserved, 1

SVC_MODE        EQU       0xD3                    ; Disable IRQ/FIQ SVC mode
IRQ_MODE        EQU       0xD2                    ; Disable IRQ/FIQ IRQ mode
FIQ_MODE        EQU       0xD1                    ; Disable IRQ/FIQ FIQ mode
ABT_MODE        EQU       0xD7                    ; Disable IRQ/FIQ ABORT mode
UND_MODE        EQU       0xDB                    ; Disable IRQ/FIQ UNDEFINE mode
SYS_MODE        EQU       0xDF                    ; Disable IRQ/FIQ SYS mode
;
;
    IMPORT  _tx_thread_system_stack_ptr
    IMPORT  _tx_initialize_unused_memory
    IMPORT  _tx_thread_context_save
    IMPORT  _tx_thread_context_restore
    IMPORT  _tx_timer_interrupt
    IMPORT  aic_undef_handler
    IMPORT  aic_swi_handler
    IMPORT  aic_dabt_handler
    IMPORT  irq_handler
    IMPORT  fiq_handler
    IMPORT  cur_mode_reg
    IMPORT  svc_mode_reg
    IMPORT  und_mode_reg
    IMPORT  abt_mode_reg
    IMPORT  irq_mode_reg
    IMPORT  fiq_mode_reg
    IMPORT  dabt_reg
    IMPORT  pabt_reg
    IMPORT  assert_handle_und
    IMPORT  assert_handle_pabt
    IMPORT  assert_handle_dabt
    IMPORT  assert_handle_swi
    IMPORT  assert_handle_resrv

    IMPORT  g_svc_stack_limit
    IMPORT  |Image$$DATA$$ZI$$Limit|

    MACRO
    SAVE_ALL_REGS
    STMFD   r13!, {r12}                 ; Push R12
    LDR     r12, =cur_mode_reg          ; Get saved area address
    STMIA   r12, {r0 - r11}             ; Save R0 ~ R11 to saved area
    LDMFD   r13!, {r0}                  ; Pop R12
    STR     r0, [r12, #48]              ; Save R12
    STR     r13, [r12, #52]             ; Save R13
    STR     r14, [r12, #56]             ; Save R14
    MOV     r0, pc
    STR     r0, [r12, #60]              ; Save PC
    MRS     r0, spsr
    STR     r0, [r12, #64]              ; Save SPSR
    MRS     r0, cpsr
    STR     r0, [r12, #68]              ; Save CPSR

    MRS     r1, cpsr                    ; Save current mode
    MOV     r0, #SVC_MODE
    MSR     cpsr_c, r0                  ; Switch to SVC mode
    LDR     r12, =svc_mode_reg          ; Get saved area address
    STR     r13, [r12, #0]              ; Save R13_svc
    STR     r14, [r12, #4]              ; Save R14_svc
    MRS     r0, spsr
    STR     r0, [r12, #8]               ; Save SPSR_svc

    MOV     r0, #IRQ_MODE
    MSR     cpsr_c, r0                  ; Switch to IRQ mode
    LDR     r12, =irq_mode_reg          ; Get saved area address
    STR     r13, [r12, #0]              ; Save R13_irq
    STR     r14, [r12, #4]              ; Save R14_irq
    MRS     r0, spsr
    STR     r0, [r12, #8]               ; Save SPSR_irq

    MOV     r0, #ABT_MODE
    MSR     cpsr_c, r0                  ; Switch to ABT mode
    LDR     r12, =abt_mode_reg          ; Get saved area address
    STR     r13, [r12, #0]              ; Save R13_abt
    STR     r14, [r12, #4]              ; Save R14_abt
    MRS     r0, spsr
    STR     r0, [r12, #8]               ; Save SPSR_abt

    MOV     r0, #UND_MODE
    MSR     cpsr_c, r0                  ; Switch to und mode
    LDR     r12, =und_mode_reg          ; Get saved area address
    STR     r13, [r12, #0]              ; Save R13_und
    STR     r14, [r12, #4]              ; Save R14_und
    MRS     r0, spsr
    STR     r0, [r12, #8]               ; Save SPSR_und

    MOV     r0, #FIQ_MODE
    MSR     cpsr_c, r0                  ; Switch to fiq mode
    LDR     r12, =fiq_mode_reg          ; Get saved area address
    STR     r13, [r12, #0]              ; Save R13_fiq
    STR     r14, [r12, #4]              ; Save R14_fiq
    MRS     r0, spsr
    STR     r0, [r12, #8]               ; Save SPSR_fiq

    MSR     cpsr_c, r1                  ; Switch to current mode
    LTORG
    MEND


    AREA ||.text||, CODE, READONLY
    PRESERVE8
;/**************************************************************************/
;/*                                                                        */
;/*  FUNCTION                                               RELEASE        */
;/*                                                                        */
;/*    _tx_initialize_low_level                          Cortex-R5/AC5     */
;/*                                                            6.1         */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */
;/*    This function is responsible for any low-level processor            */
;/*    initialization, including setting up interrupt vectors, setting     */
;/*    up a periodic timer interrupt source, saving the system stack       */
;/*    pointer for use in ISR processing later, and finding the first      */
;/*    available RAM memory address for tx_application_define.             */
;/*                                                                        */
;/*  INPUT                                                                 */
;/*                                                                        */
;/*    None                                                                */
;/*                                                                        */
;/*  OUTPUT                                                                */
;/*                                                                        */
;/*    None                                                                */
;/*                                                                        */
;/*  CALLS                                                                 */
;/*                                                                        */
;/*    None                                                                */
;/*                                                                        */
;/*  CALLED BY                                                             */
;/*                                                                        */
;/*    _tx_initialize_kernel_enter           ThreadX entry function        */
;/*                                                                        */
;/*  RELEASE HISTORY                                                       */
;/*                                                                        */
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_initialize_low_level(VOID)
;{
    EXPORT _tx_initialize_low_level
_tx_initialize_low_level
;
;    /* We must be in SVC mode at this point!  */
;
;    /* Save the system stack pointer.  */
;    _tx_thread_system_stack_ptr = (VOID_PTR) (sp);
;
    LDR     r3, = g_svc_stack_limit  ; Get pointer to stack area
    LDR     r1, [r3]
    LDR     r2, =_tx_thread_system_stack_ptr    ; Pickup stack pointer
    STR     r1, [r2]                            ; Save the system stack
;
;    /* Save the first available memory address.  */
;    _tx_initialize_unused_memory =  (VOID_PTR) _end;
;
    LDR     r1, =|Image$$DATA$$ZI$$Limit|       ; Get end of non-initialized RAM area
    LDR     r2, =_tx_initialize_unused_memory   ; Pickup unused memory ptr address
    ADD     r1, r1, #8                          ; Increment to next free word
    STR     r1, [r2]                            ; Save first free memory address
;
;    /* Setup Timer for periodic interrupts.  */
;
;    /* Done, return to caller.  */
;
    IF :DEF:__THUMB_INTERWORK
    BX      lr                                  ; Return to caller
    ELSE
    MOV     pc, lr                              ; Return to caller
    ENDIF
    LTORG
;}
;

    EXPORT __save_all_regs
__save_all_regs
    SAVE_ALL_REGS

;   /* Return to caller. */
    BX      lr

;
;/* Define shells for each of the interrupt vectors.  */
;
    EXPORT __tx_undefined
__tx_undefined
    STMDB   sp!, {r0-r3, r12, lr}
    MOV     r0, r14
    MRS     r1, spsr
    BLX     aic_undef_handler
    LDMIA   sp!, {r0-r3, r12, lr}

    BLX     __save_all_regs

    LDR     r0, =assert_handle_und
    BX      r0

;
    EXPORT __tx_swi_interrupt
__tx_swi_interrupt
    STMDB   sp!, {r0-r3, r12, lr}
    BLX     aic_swi_handler
    LDMIA   sp!, {r0-r3, r12, lr}

    BLX     __save_all_regs

    LDR     r0, =assert_handle_swi
    BX      r0

;
    IMPORT  aic_pabt_handler
    EXPORT  __tx_prefetch_handler
__tx_prefetch_handler
    STMDB   sp!, {r0-r3, r12, lr}
    MOV     r0, r14
    MRC     p15, 0, r1, c5, c0, 1
    BLX     aic_pabt_handler
    LDMIA   sp!, {r0-r3, r12, lr}

    BLX     __save_all_regs

    LDR     r0, =pabt_reg
    MRC     p15, 0, r1, c6, c0, 2               ; Ins fault address
    STR     r1, [r0, #0]
    MRC     p15, 0, r1, c5, c0, 1               ; Ins fault status
    STR     r1, [r0, #4]
    MRC     p15, 0, r1, c5, c1, 1               ; Aux ins fault status
    STR     r1, [r0, #8]

    LDR     r0, =assert_handle_pabt
    BX      r0

;
    EXPORT __tx_abort_handler
__tx_abort_handler
    STMDB   sp!, {r0-r3, r12, lr}
    MRC     p15, 0, r0, c5, c0, 0
    MRC     p15, 0, r1, c6, c0, 0
    BLX     aic_dabt_handler
    LDMIA   sp!, {r0-r3, r12, lr}

    BLX     __save_all_regs

    LDR     r0, =dabt_reg
    MRC     p15, 0, r1, c6, c0, 0               ; Data fault address
    STR     r1, [r0, #0]
    MRC     p15, 0, r1, c5, c0, 0               ; Data fault status
    STR     r1, [r0, #4]
    MRC     p15, 0, r1, c5, c1, 0               ; Aux data fault status
    STR     r1, [r0, #8]

    LDR     r0, =assert_handle_dabt
    BX      r0

;
    EXPORT __tx_reserved_handler
__tx_reserved_handler
    BLX     __save_all_regs

    LDR     r0, =assert_handle_resrv
    BX      r0
    LTORG
;
    EXPORT __tx_irq_processing_return
    EXPORT __tx_irq_handler
__tx_irq_handler
;
;    /* Jump to context save to save system context.  */
    B       _tx_thread_context_save
__tx_irq_processing_return
;
;    /* At this point execution is still in the IRQ mode.  The CPSR, point of
;       interrupt, and all C scratch registers are available for use.  In
;       addition, IRQ interrupts may be re-enabled - with certain restrictions -
;       if nested IRQ interrupts are desired.  Interrupts may be re-enabled over
;       small code sequences where lr is saved before enabling interrupts and
;       restored after interrupts are again disabled.  */
;
;    /* Interrupt nesting is allowed after calling _tx_thread_irq_nesting_start
;       from IRQ mode with interrupts disabled.  This routine switches to the
;       system mode and returns with IRQ interrupts enabled.
;
;       NOTE:  It is very important to ensure all IRQ interrupts are cleared
;       prior to enabling nested IRQ interrupts.  */
    IF :DEF:TX_ENABLE_IRQ_NESTING
    BL      _tx_thread_irq_nesting_start
    ENDIF
;
;    /* Process platform specified interrupt */
    BLX    irq_handler

;    /* For debug purpose, execute the timer interrupt processing here.  In
;       a real system, some kind of status indication would have to be checked
;       before the timer interrupt handler could be called.  */
;
;    BL     _tx_timer_interrupt                  ; Timer interrupt handler
;
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_context_restore.
;       This routine returns in processing in IRQ mode with interrupts disabled.  */
    IF :DEF:TX_ENABLE_IRQ_NESTING
    BL      _tx_thread_irq_nesting_end
    ENDIF
;
;    /* Jump to context restore to restore system context.  */
    B       _tx_thread_context_restore
;
;
;    /* This is an example of a vectored IRQ handler.  */
;
;    EXPORT __tx_example_vectored_irq_handler
;__tx_example_vectored_irq_handler
;
;
;    /* Save initial context and call context save to prepare for
;       vectored ISR execution.  */
;
;    STMDB   sp!, {r0-r3}                        ; Save some scratch registers
;    MRS     r0, SPSR                            ; Pickup saved SPSR
;    SUB     lr, lr, #4                          ; Adjust point of interrupt
;    STMDB   sp!, {r0, r10, r12, lr}             ; Store other scratch registers
;    BL      _tx_thread_vectored_context_save    ; Vectored context save
;
;    /* At this point execution is still in the IRQ mode.  The CPSR, point of
;       interrupt, and all C scratch registers are available for use.  In
;       addition, IRQ interrupts may be re-enabled - with certain restrictions -
;       if nested IRQ interrupts are desired.  Interrupts may be re-enabled over
;       small code sequences where lr is saved before enabling interrupts and
;       restored after interrupts are again disabled.  */
;
;
;    /* Interrupt nesting is allowed after calling _tx_thread_irq_nesting_start
;       from IRQ mode with interrupts disabled.  This routine switches to the
;       system mode and returns with IRQ interrupts enabled.
;
;       NOTE:  It is very important to ensure all IRQ interrupts are cleared
;       prior to enabling nested IRQ interrupts.  */
;    IF :DEF:TX_ENABLE_IRQ_NESTING
;    BL      _tx_thread_irq_nesting_start
;    ENDIF
;
;    /* Application IRQ handlers can be called here!  */
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_context_restore.
;       This routine returns in processing in IRQ mode with interrupts disabled.  */
;    IF :DEF:TX_ENABLE_IRQ_NESTING
;    BL      _tx_thread_irq_nesting_end
;    ENDIF
;
;    /* Jump to context restore to restore system context.  */
;    B       _tx_thread_context_restore
;
;
    IF :DEF:TX_ENABLE_FIQ_SUPPORT
    EXPORT  __tx_fiq_handler
    EXPORT  __tx_fiq_processing_return
__tx_fiq_handler
;
;    /* Jump to fiq context save to save system context.  */
    B       _tx_thread_fiq_context_save

__tx_fiq_processing_return
;
;    /* At this point execution is still in the FIQ mode.  The CPSR, point of
;       interrupt, and all C scratch registers are available for use.  */
;
;    /* Interrupt nesting is allowed after calling _tx_thread_fiq_nesting_start
;       from FIQ mode with interrupts disabled.  This routine switches to the
;       system mode and returns with FIQ interrupts enabled.
;
;       NOTE:  It is very important to ensure all FIQ interrupts are cleared
;       prior to enabling nested FIQ interrupts.  */
    IF :DEF:TX_ENABLE_FIQ_NESTING
    BL      _tx_thread_fiq_nesting_start
    ENDIF
;
;    /* Application FIQ handlers can be called here!  */
;    /* Process platform specified interrupt */
    BL aic_fiq_handler
;
;    /* If interrupt nesting was started earlier, the end of interrupt nesting
;       service must be called before returning to _tx_thread_fiq_context_restore.  */
    IF :DEF:TX_ENABLE_FIQ_NESTING
    BL      _tx_thread_fiq_nesting_end
    ENDIF
;
;    /* Jump to fiq context restore to restore system context.  */
    B       _tx_thread_fiq_context_restore
;
;
    ELSE
    EXPORT  __tx_fiq_handler
__tx_fiq_handler
    stmdb sp!, {r0-r3, r12, lr}
    blx fiq_handler
    ldmia sp!, {r0-r3, r12,lr}
    movs pc, lr
    ENDIF
;
;
large_table
    SPACE   4200
    IMPORT  _tx_build_options
    IMPORT  _tx_version_id
BUILD_OPTIONS
    DCD     _tx_build_options                    ; Reference to bring in
VERSION_ID
    DCD     _tx_version_id                       ; Reference to bring in

    ALIGN
    LTORG
    END
