/* vex_cmd.h
 *
 * Driver Command/Response data structure.
 * Used on character device interface between Vex Proxy and Stub.
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

#ifndef INCLUDE_GUARD_VEX_CMD_H
#define INCLUDE_GUARD_VEX_CMD_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// UMDevXS Command API
#include "umdevxs_cmd.h"        // UMDevXS_CmdRsp_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

enum
{
    VEX_OPCODE_TOKEN = UMDEVXS_OPCODE_LAST + 1,

    VEX_OPCODE_LAST,
};


typedef struct
{
    // Pointer to input data
    void * InData_p;

    // Input Data byte count
    unsigned int InDataByteCount;

    // Pointer to input data
    void * OutData_p;

    // Output Data byte count
    unsigned int OutDataByteCount;

} vexStub_DataIO_t;


/*----------------------------------------------------------------------------
 * Local vexStub_HandleCmd
 */
int
vexStub_HandleCmd(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p);


#endif /* INCLUDE_GUARD_VEX_CMD_H */

/* vex_cmd.h */
