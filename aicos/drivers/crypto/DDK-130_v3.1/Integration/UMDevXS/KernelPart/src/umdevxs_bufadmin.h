/* umdevxs_bufadmin.h
 *
 * Support code that provides a 'BufAdmin_Record_t' type for storing
 * implementation-specific information on buffers and a set of functions
 * to manage the storage of such records and provide handle-based
 * access to them.
 *
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

#ifndef INCLUDE_GUARD_UMDEVXS_BUFADMIN_H
#define INCLUDE_GUARD_UMDEVXS_BUFADMIN_H

#include "basic_defs.h"         // bool, uint32_t, inline


/*----------------------------------------------------------------------------
 * BufAdmin_Record_t
 *
 * This type is the record that describes a buffer. The caller shall use
 * BufAdmin_Record_Create to instantiate a record and get a handle to that
 * record.
 * Use BufAdmin_Handle2RecordPtr to get a pointer to the actual
 * record (as shown below) and to manipulate the fields in the record.
 */
typedef struct
{
    uint32_t Magic;     // signature used to validate handles

    void * AppID;       // tracks which app this belongs to, for clean-up

    struct
    {
        // for freeing the buffer
        void * AllocatedAddr_p;
        unsigned int AllocatedSize;     // in bytes

        void * Alternative_p;
        //char AllocatorRef;

        // for separating SoC memory from main memory
        uint8_t MemoryBank;

        // Physical address
        void * PhysAddr_p;
    } alloc;

    struct
    {
        // alignment used for HostAddr_p
        unsigned int Alignment;

        // aligned start-address, data starts here
        void * HostAddr_p;

        // maximum data amount that can be stored from HostAddr_p
        unsigned int BufferSize;        // in bytes

        // true = memory is cached
        //bool fCached;
    } host;

    struct
    {
        void * Addr;
        unsigned int Size;
    } user;

} BufAdmin_Record_t;


/*----------------------------------------------------------------------------
 * BufAdmin_Handle_t
 *
 * The handle type that references a certain BufAdmin_Record_t record.
 * Use BufAdmin_Handle2RecordPtr to get a pointer to the actual
 * record (as shown below) and to manipulate the fields in the record.
 */
typedef int BufAdmin_Handle_t;

#define BUFADMIN_HANDLE_NULL  0


/*----------------------------------------------------------------------------
 * BufAdmin_Init
 *
 * This function must be used to initialize the buffer record administration.
 * It must be called before any of the other BufAdmin_* functions may
 * be called. It may be called anew only after BufAdmin_UnInit has
 * been called.
 *
 * MaxHandles (input)
 *     Maximum number of handles that must be possible to administrate. The
 *     implementation will allocate enough memory to fulfill this requirement.
 *     Set to 0 to force the implementation to use the memory provided via
 *     AdminMemory_p and AdminMemorySize.
 *
 * AdminMemory_p (input)
 *     This parameter is used only when MaxHandles is 0.
 *     Pointer to the block of memory this implementation may use for the
 *     records.
 *
 * AdminMemorySize (input)
 *     This parameter is used only when MaxHandles is 0.
 *     Size of the memory block pointed to by AdminMemory_p.
 *     The implementation will calculate the maximum number of handles it can
 *     administrate.
 *
 * Return Value
 *     true   Initialization successfully, rest of the API may now be used.
 *     false  Initialization failed.
 */
bool
BufAdmin_Init(
        const unsigned int MaxHandles,
        void * AdminMemory_p,
        const unsigned int AdminMemorySize);


/*----------------------------------------------------------------------------
 * BufAdmin_UnInit
 *
 * This function can be used to uninitialize the buffer record administration.
 * The caller must make sure that handles will not be used after this function
 * returns. If memory was allocated by BufAdmin_Init, this function
 * will free it.
 */
void
BufAdmin_UnInit(void);


/*----------------------------------------------------------------------------
 * BufAdmin_Record_Create
 *
 * This function can be used to create a record. The function returns a handle
 * for the record. Use BufAdmin_Handle2RecordPtr to access the record.
 * Destroy the record when no longer required, see BufAdmin_Record_Destroy.
 * This function initializes the record to all zeros.
 *
 * Return Values
 *     Handle for the buffer record.
 *     NULL is returned when the creation failed.
 */
BufAdmin_Handle_t
BufAdmin_Record_Create(void);


/*----------------------------------------------------------------------------
 * BufAdmin_Record_Destroy
 *
 * This function can be used to delete a record. This deletes all the fields
 * and invalidates the handle.
 *
 * Handle
 *     A valid handle that was once returned by BufAdmin_Record_Create.
 *
 * Return Values
 *     None
 */
void
BufAdmin_Record_Destroy(
        BufAdmin_Handle_t Handle);


/*----------------------------------------------------------------------------
 * BufAdmin_IsValidHandle
 *
 * This function validates a handle.
 *
 * Handle
 *     A valid handle that was once returned by BufAdmin_Record_Create.
 *
 * Return Value
 *     true  The handle is valid
 *     false The handle is NOT valid
 */
bool
BufAdmin_IsValidHandle(
        BufAdmin_Handle_t Handle);


/*----------------------------------------------------------------------------
 * BufAdmin_Handle2RecordPtr
 *
 * This function can be used to get a pointer to the buffer record
 * (BufAdmin_Record_t) for the handle. The pointer is valid until
 * the handle is destroyed with BufAdmin_Record_Destroy.
 *
 * Handle
 *     A valid handle that was once returned by BufAdmin_Record_Create.
 *
 * Return Value
 *     Pointer to the BufAdmin_Record_t memory for this handle.
 *     NULL is returned if the handle is invalid.
 */
BufAdmin_Record_t *
BufAdmin_Handle2RecordPtr(
        BufAdmin_Handle_t Handle);


/*----------------------------------------------------------------------------
 * BufAdmin_Enumerate
 *
 * This function iterates through all existing records and invokes the
 * provided function for each record.
 */

typedef void (* BufAdmin_EnumFnc_t)(
        BufAdmin_Handle_t Handle,
        BufAdmin_Record_t * const Rec_p,
        void * Param1);

void
BufAdmin_Enumerate(
        BufAdmin_EnumFnc_t EnumFnc,
        void * Param1);


#endif /* INCLUDE_GUARD_UMDEVXS_BUFADMIN_H */

/* end of file umdevxs_bufadmin.h */
