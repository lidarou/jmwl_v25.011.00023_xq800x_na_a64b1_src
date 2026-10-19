/* adapter_interrupts.h
 *
 * Adapter Interrupts interface
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : DDK-130_bsd                                              */
/*   Version       : 3.1                                                      */
/*   Configuration : DDK_EIP130_BSD                                           */
/*                                                                            */
/*   Date          : 2022-Feb-14                                              */
/*                                                                            */
/* Copyright (c) 2014-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or without         */
/* modification, are permitted provided that the following conditions are     */
/* met:                                                                       */
/*                                                                            */
/* 1. Redistributions of source code must retain the above copyright          */
/* notice, this list of conditions and the following disclaimer.              */
/*                                                                            */
/* 2. Redistributions in binary form must reproduce the above copyright       */
/* notice, this list of conditions and the following disclaimer in the        */
/* documentation and/or other materials provided with the distribution.       */
/*                                                                            */
/* 3. Neither the name of the copyright holder nor the names of its           */
/* contributors may be used to endorse or promote products derived from       */
/* this software without specific prior written permission.                   */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR      */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       */
/* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT           */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,      */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE      */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.       */
/* -------------------------------------------------------------------------- */

#ifndef INCLUDE_GUARD_ADAPTER_INTERRUPTS_H
#define INCLUDE_GUARD_ADAPTER_INTERRUPTS_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Defs API
#include "basic_defs.h"     // bool


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef void (* Adapter_InterruptHandler_t)(const int nIRQ, const unsigned int Flags);


/*----------------------------------------------------------------------------
 *                           Adapter_Interrupts
 *----------------------------------------------------------------------------
 */

int
Adapter_Interrupts_Init(
        const int nIRQ);

int
Adapter_Interrupts_UnInit(
        const int nIRQ);

int
Adapter_Interrupt_SetHandler(
        const int nIRQ,
        Adapter_InterruptHandler_t HandlerFunction);

int
Adapter_Interrupt_Enable(
        const int nIRQ,
        const unsigned int Flags);

/*
 * This function enables execution context (allows it to run) that services
 * the nIRQ interrupt.
 *
 * Note: this function is different from the Adapter_Interrupt_Enable()
 *       function which only enables the nIRQ interrupt event at an interrupt
 *       controller. The interrupt can be dispatched to some execution context
 *       (for example, a thread) that can be also enabled or disabled.
 */
int
Adapter_Interrupt_Context_Enable(
        const int nIRQ,
        const unsigned int Flags);

int
Adapter_Interrupt_Clear(
        const int nIRQ,
        const unsigned int Flags);

int
Adapter_Interrupt_ClearAndEnable(
        const int nIRQ,
        const unsigned int Flags);

int
Adapter_Interrupt_Disable(
        const int nIRQ,
        const unsigned int Flags);

/*
 * This function disables execution context (allows it to run) that services
 * the nIRQ interrupt.
 *
 * Note: this function is different from the Adapter_Interrupt_Disable()
 *       function which only disables the nIRQ interrupt event at an interrupt
 *       controller. The interrupt can be dispatched to some execution context
 *       (for example, a thread) that can be also enabled or disabled.
 */
int
Adapter_Interrupt_Context_Disable(
        const int nIRQ,
        const unsigned int Flags);

int
Adapter_Interrupts_Resume(void);


// Host hardware specific extensions
#include "adapter_interrupts_ext.h"


#endif /* INCLUDE_GUARD_ADAPTER_INTERRUPTS_H */

/* end of file adapter_interrupts.h */
