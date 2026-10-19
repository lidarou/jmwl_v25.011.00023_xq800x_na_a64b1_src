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

    IMPORT      _start
    EXPORT      reset_handler 
    IMPORT      __tx_undefined
    IMPORT      __tx_swi_interrupt
    IMPORT      __tx_prefetch_handler
    IMPORT      __tx_abort_handler
    IMPORT      __tx_reserved_handler
    IMPORT      __tx_irq_handler
    IMPORT      __tx_fiq_handler
;
;
;/* Define the vector area.  This should be located or copied to 0.  */
;
    AREA ||.vectors||, CODE, READONLY

    EXPORT __vectors
reset_handler
__vectors

    LDR     pc, STARTUP                     ; Reset goes to startup function
    LDR     pc, UNDEFINED                   ; Undefined handler
    LDR     pc, SWI                         ; Software interrupt handler
    LDR     pc, PREFETCH                    ; Prefetch exception handler
    LDR     pc, ABORT                       ; Abort exception handler
    LDR     pc, RESERVED                    ; Reserved exception handler
    LDR     pc, IRQ                         ; IRQ interrupt handler
    LDR     pc, FIQ                         ; FIQ interrupt handler

STARTUP
    DCD     _start                           ; Reset goes to C startup function
UNDEFINED
    DCD     __tx_undefined                   ; Undefined handler
SWI
    DCD     __tx_swi_interrupt               ; Software interrupt handler
PREFETCH
    DCD     __tx_prefetch_handler            ; Prefetch exception handler
ABORT
    DCD     __tx_abort_handler               ; Abort exception handler
RESERVED                            
    DCD     __tx_reserved_handler            ; Reserved exception handler
IRQ
    DCD     __tx_irq_handler                 ; IRQ interrupt handler
FIQ
    DCD     __tx_fiq_handler                 ; FIQ interrupt handler

    END
