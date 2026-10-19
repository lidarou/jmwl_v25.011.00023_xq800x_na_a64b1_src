/* umdevxsproxy_interrupt.h
 *
 * This user-mode library handles the communication with the
 * Linux User Mode Device Access (UMDevXS) driver.
 * Using this part of the API it is possible to wait for an interrupt event.
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

#ifndef INCLUDE_GUARD_UMDEVXSPROXY_INTERRUPT_H
#define INCLUDE_GUARD_UMDEVXSPROXY_INTERRUPT_H


/*----------------------------------------------------------------------------
 * UMDevXSProxy_Interrupt_WaitWithTimeout
 *
 * This function must be used to wait for an interrupt even to occur. As soon
 * as the interrupt has been reported (by the OS), the function call returns.
 * The timeout value requests a maximum wait duration, in milliseconds.
 *
 * Timeout_ms (input)
 *     Maximum time to wait for the interrupt. If the interrupt does not occur
 *     when this amount of time has elapsed, the function returns and reports
 *     the timeout instead of interrupt occurrence.
 *
 * IntId (input)
 *     Interrupt identifier for which the wait timeout is specified.
 *
 * Return Value
 *     0  Return due to interrupt
 *     1  Return due to timeout
 *    <0  Error code
 */
int
UMDevXSProxy_Interrupt_WaitWithTimeout(
        const unsigned int Timeout_ms,
        const int IntId);


#endif /* INCLUDE_GUARD_UMDEVXSPROXY_INTERRUPT_H */

/* end of file umdevxsproxy_interrupt.h */
