/**
 * @file api_val_nop.h
 *
 * @brief VAL API - NOP service
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

#ifndef INCLUDE_GUARD_API_VAL_NOP_H
#define INCLUDE_GUARD_API_VAL_NOP_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"             // uint8_t, uint16_t, uint32_t, uint64_t

#include "api_val_result.h"         // ValStatus_t
#include "api_val_buffers.h"        // ValOctetsIn_t, ValOctetsOut_t


/*-----------------------------------------------------------------------------
 * val_NOP
 */
/**
 * This function allows data to be copied with the DMA engine.
 *
 * @param [in] SrcData_p
 *     Pointer to the buffer with the data that needs to be copied.
 *
 *
 * @param [in] DstData_p
 *     Pointer to the buffer in which the data needs to be copied.
 *
 * @param [in] DataSize
 *     The size of data to be copied.
 *     Note that the size is equal for the Source and Destination Data and
 *     will be round-up to a multiple of 4 bytes.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_NOP(
        ValOctetsIn_t * const  SrcData_p,
        ValOctetsOut_t * const DstData_p,
        const ValSize_t  DataSize);


#endif /* INCLUDE_GUARD_API_VAL_NOP_H */

/* end of file api_val_nop.h */
