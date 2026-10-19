/* umdevxs_cmd.h
 *
 * Driver Command/Response data structure.
 * Used on character device interface between Kernel Driver and Proxy.
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

#ifndef INCLUDE_GUARD_UMDEVXS_CMD_H
#define INCLUDE_GUARD_UMDEVXS_CMD_H

// most operations are done through

enum
{
    UMDEVXS_OPCODE_INIT = 1,
    UMDEVXS_OPCODE_SHUTDOWN,

    UMDEVXS_OPCODE_SMBUF_ALLOC,
    // Alloc:
    //  In: Size(uint1), Bank(uint2), Alignment(uint3)
    // Out: Handle(Handle), ActualSize(uint1), DevAddr(ptr1)
    // currently only supports PAGE_SIZE-aligned, DMA-capable buffers.

    UMDEVXS_OPCODE_SMBUF_SETBUFINFO,
    // GetBufInfo:
    //  In: Handle, Size(uint1), Addr(ptr1)

    UMDEVXS_OPCODE_SMBUF_GETBUFINFO,
    // GetBufInfo:
    //  In: Handle
    // Out: Size(uint1), Addr(ptr1)

    UMDEVXS_OPCODE_SMBUF_REGISTER,
    // Register:
    //  In: Size(uint1), BufPtr(ptr1), Handle(Handle)
    // Out: NewHandle(Handle)
    // input handle must refer to an already allocated/attached buffer that
    // includes the registered buffer.

    UMDEVXS_OPCODE_SMBUF_FREE,
    // Free:
    //  In: Handle

    UMDEVXS_OPCODE_SMBUF_ATTACH,
    // Attach:
    //  In: DevAddr(ptr1), Size(uint1), Bank(uint2)
    // Out: Handle, Size(uint1)

    UMDEVXS_OPCODE_SMBUF_DETACH,
    // Detach:
    //  In: Handle

    UMDEVXS_OPCODE_SMBUF_COMMIT,
    // Commit:
    //  In: Handle, SubsetStart(uint1), SubsetLength(uint2)

    UMDEVXS_OPCODE_SMBUF_REFRESH,
    // Refresh:
    //  In: Handle, SubsetStart(uint1), SubsetLength(uint2)

    UMDEVXS_OPCODE_DEVICE_FIND,
    // Find:
    //   In: szName
    //  Out: Handle, Size(uint1)

    UMDEVXS_OPCODE_DEVICE_FINDRANGE,
    // Find device specified by range:
    //   In: BAR (uint1), Offset (unit2), Size (uint3)
    //  Out: Handle

    UMDEVXS_OPCODE_DEVICE_SETPRIV,
    // SetPrivileged.

    UMDEVXS_OPCODE_DEVICE_ENUM,
    // Enum:
    //   In: DeviceNr(uint1)
    //  Out: szName

    UMDEVXS_OPCODE_DEVICE_PCICFG_READ32,
    // PCI Cfg Read32:
    //   In: ByteOffset (uint1)
    //  Out: Int32 (uint2)

    UMDEVXS_OPCODE_DEVICE_PCICFG_WRITE32,
    // PCI Cfg Write32:
    //   In: ByteOffset (uint1), Int32 (uint2)


    UMDEVXS_OPCODE_LAST,
};


#define UMDEVXS_CMDRSP_MAXLEN_NAME 64

typedef struct
{
    int Magic;                  // in
    int Opcode;                 // in
    int Error;                  // out, 0 = no error

    int Handle;                 // in

    unsigned int uint1;
    unsigned int uint2;
    unsigned int uint3;
    void * ptr1;
    char szName[UMDEVXS_CMDRSP_MAXLEN_NAME + 1];  // zero-terminated

} UMDevXS_CmdRsp_t;


#define UMDEVXS_CMDRSP_MAGIC 876327949

#define UMDEVXS_INTERRUPT_TIMEOUT_MASK      0x00FFFFFFU
#define UMDEVXS_INTERRUPT_INT_ID_MASK       0x000000FFU
#define UMDEVXS_INTERRUPT_INT_ID_OFFSET     24


#endif /* INCLUDE_GUARD_UMDEVXS_CMD_H */

/* end of file umdevxs_cmd.h */
