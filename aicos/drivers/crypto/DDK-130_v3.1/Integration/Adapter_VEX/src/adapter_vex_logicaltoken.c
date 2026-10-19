/* adapter_vex_logicaltoken.c
 *
 * Implementation of the VEX API.
 *
 * This file contains the logical token exchange implementation between the
 * VAL API and VEX API.
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

#include "c_adapter_vex.h"          // configuration

#include "basic_defs.h"             // uint16_t, uint32_t, bool
#include "clib.h"
#include "log.h"                    // LOG_*()

#include "adapter_vex.h"            // API to implement
#include "adapter_vex_internal.h"   // vex_*() and VexToken_Command/Result_t
#include "adapter_alloc.h"          // Adapter_Alloc() and Adapter_Free()
#include "adapter_lock.h"           // Adapter_Lock_t, Adapter_Lock*()
#include "adapter_event.h"          // Adapter_Event_t, Adapter_Event*()
#include "adapter_thread.h"         // Adapter_ThreadStart/Stop/ShouldStop()
#include "adapter_sleep.h"          // Adapter_SleepMS()
#include "adapter_getpid.h"         // Adapter_GetPid()
#include "dmares_types.h"           // DMAResource_Handle_t/Properties_t/AddrPair_t
#include "dmares_buf.h"             // DMAResource_Alloc/Release
#ifdef MODULE
#include <linux/uaccess.h>          // copy_to_user, copy_from_user
#endif

#define ADAPTER_VEX_THREAD_SLEEP_MS             50
#define ADAPTER_VEX_THREAD_REQUEST_TIMEOUT_MS   10000

#define ADAPTER_VEX_BUFFER_ENTRY_MAGIC          0x56657842  /* VexB */
#define ADAPTER_VEX_BUFFER_PREALLOC_SIZE        16

/*----------------------------------------------------------------------------
 * VexBufferRequestStatus_t
 */
typedef enum
{
    INIT = 0,                           /* Initial state */
    READY = 1,                          /* Request ready to be excuted */
    EXECUTING = 2,                      /* Request is being excuted */
    EXECUTED = 3,                       /* Request excuted - waiting for result processing */
} BufferEntryStatus_t;

/*----------------------------------------------------------------------------
 * VexBufferRequestStatus_t
 */
struct BufferEntry
{
    volatile struct BufferEntry * pNext;
    volatile struct BufferEntry * pPrev;
    VexToken_Command_t * CommandToken_p;
    VexToken_Result_t * ResultToken_p;
    Adapter_Event_t ResultEvent;
    uint32_t Magic;
    VexStatus_t ReturnValue;
    volatile BufferEntryStatus_t status;
    bool Allocated;
};

static void * gl_HndTokenThread = NULL;
static Adapter_Event_t gl_RequestEvent = NULL;
static unsigned int gl_nLogicalTokens = 0;
static unsigned int gl_FIFODepth = 0;
static unsigned int gl_maxFIFODepth = 0;
static struct BufferEntry gl_Buffer[ADAPTER_VEX_BUFFER_PREALLOC_SIZE];
static volatile struct BufferEntry * gl_RequestRoot = NULL;
static volatile struct BufferEntry * gl_RequestEnd = NULL;
static volatile struct BufferEntry * gl_ResultRoot = NULL;
static volatile struct BufferEntry * gl_FreeRoot = NULL;
static Adapter_Lock_t gl_AdminMutexlock = NULL;
static unsigned long gl_AdminMutexFlags = 0;

/*----------------------------------------------------------------------------
 * discard_const
 *      Discard const qualifier from pointer
 */
static inline void *
discard_const(const void * Ptr_p)
{
    union
    {
        const void * c_p;
        void * n_p;
    } Conversion;

    Conversion.c_p = Ptr_p;
    return Conversion.n_p;
}

/*----------------------------------------------------------------------------
 * discard_volatile
 *      Discard volatile qualifier from pointer
 */
static inline void *
discard_volatile(volatile void * Ptr_p)
{
    union
    {
        volatile void * v_p;
        void * n_p;
    } Conversion;

    Conversion.v_p = Ptr_p;
    return Conversion.n_p;
}

/*----------------------------------------------------------------------------
 * vexLocal_RequestEval
 *      Check if there is a request.
 */
static int
vexLocal_RequestEval(void *pCpntext)
{
    IDENTIFIER_NOT_USED(pCpntext);
#ifndef MODULE
    Adapter_ThreadShouldStop();
#endif
    return (gl_RequestRoot != NULL);
}

/*----------------------------------------------------------------------------
 * vexLocal_ResultEval
 *      Check if there is a result.
 */
static int
vexLocal_ResultEval(void *pContext)
{
    return (((struct BufferEntry *)pContext)->status == EXECUTED);
}

/*----------------------------------------------------------------------------
 * vexLocal__RequestWrite
 */
static volatile struct BufferEntry *
vexLocal_RequestWrite(VexToken_Command_t * const CommandToken_p,
                      VexToken_Result_t * const ResultToken_p)
{
    volatile struct BufferEntry * pEntry;

    /* Get a free entry */
    Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
    Adapter_ThreadMemoryBarrierRead();
    pEntry = gl_FreeRoot;
    if (pEntry == NULL)
    {
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);

        pEntry = Adapter_Alloc(sizeof(struct BufferEntry));
        if (pEntry != NULL)
        {
            pEntry->Allocated = true;
        }
    }
    else
    {
        gl_FreeRoot = pEntry->pNext;
        Adapter_ThreadMemoryBarrierWrite();
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
    }

    if (pEntry != NULL)
    {
        pEntry->pNext = NULL;
        pEntry->CommandToken_p = discard_const(CommandToken_p);
        pEntry->ResultToken_p = discard_const(ResultToken_p);
        pEntry->ResultEvent = Adapter_Event_Alloc();
        pEntry->Magic = ADAPTER_VEX_BUFFER_ENTRY_MAGIC;
        pEntry->ReturnValue = 0;
        pEntry->status = INIT;

        Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
        Adapter_ThreadMemoryBarrierRead();
        pEntry->pPrev = gl_RequestEnd;
        if (gl_RequestEnd != NULL)
        {
            gl_RequestEnd->pNext = pEntry;
            gl_RequestEnd = pEntry;
            Adapter_ThreadMemoryBarrierWrite();
        }
        else
        {
            gl_RequestRoot = pEntry;
            gl_RequestEnd = pEntry;
            Adapter_ThreadMemoryBarrierWrite();
            Adapter_Event_Signal(gl_RequestEvent);
        }
        gl_FIFODepth++;
        gl_nLogicalTokens++;
        if (gl_FIFODepth > gl_maxFIFODepth)
        {
            gl_maxFIFODepth = gl_FIFODepth;
        }
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
    }

    return pEntry;
}

/*----------------------------------------------------------------------------
 * vexLocal_FreeBufferEnty
 */
static void
vexLocal_FreeBufferEnty(volatile struct BufferEntry * pEntry)
{
    pEntry->pPrev = NULL;
    pEntry->CommandToken_p = NULL;
    pEntry->ResultToken_p = NULL;
    Adapter_Event_Free(pEntry->ResultEvent);
    pEntry->ResultEvent = NULL;
    pEntry->ReturnValue = 0;
    pEntry->status = INIT;

    if (pEntry->Allocated)
    {
        pEntry->pNext = NULL;
        pEntry->Magic = 0;
        pEntry->Allocated = false;
        Adapter_Free(discard_volatile(pEntry));
    }
    else
    {
        Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
        Adapter_ThreadMemoryBarrierRead();
        pEntry->pNext = gl_FreeRoot;
        gl_FreeRoot = pEntry;
        Adapter_ThreadMemoryBarrierWrite();
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
    }
}

/*----------------------------------------------------------------------------
 * vexLocal_ResultRead
 */
static bool
vexLocal_ResultRead(volatile struct BufferEntry * pEntry,
                    bool fCleanUp,
                    VexStatus_t * retValue)
{
    bool ret = true;

    Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
    Adapter_ThreadMemoryBarrierRead();
    if (pEntry->status == EXECUTED)
    {
        /* Result is available */
        *retValue = pEntry->ReturnValue;
        if (pEntry->pNext != NULL)
        {
            pEntry->pNext->pPrev = pEntry->pPrev;
        }
        if (pEntry->pPrev != NULL)
        {
            pEntry->pPrev->pNext = pEntry->pNext;
        }
        else
        {
            gl_ResultRoot = pEntry->pNext;
        }
        Adapter_ThreadMemoryBarrierWrite();
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
        vexLocal_FreeBufferEnty(pEntry);
    }
    else if (fCleanUp)
    {
        LOG_CRIT(VEX_LOG_PREFIX "%s(%d) Cleanup due to TIMEOUT!\n",
                 __func__, Adapter_GetPid());

        Adapter_ThreadMemoryBarrierRead();
        if (pEntry->status == EXECUTING)
        {
            int display = 0;

            /* Wait until it is executed */
            do
            {
                Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
                if (Adapter_Event_Wait(pEntry->ResultEvent,
                                       vexLocal_ResultEval,
                                       discard_volatile(pEntry),
                                       ADAPTER_VEX_THREAD_SLEEP_MS) == 0)
                {
                    if ((display % (1000/ADAPTER_VEX_THREAD_SLEEP_MS)) == 0)
                    {
                        LOG_CRIT(VEX_LOG_PREFIX "%s(%d) Wait EXECUTING TIMEOUT!\n",
                                 __func__, Adapter_GetPid());
                    }
                    display++;
                }
                Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
                Adapter_ThreadMemoryBarrierRead();
            } while (pEntry->status == EXECUTING);

            /* Result is available */
            *retValue = pEntry->ReturnValue;
            if (pEntry->pNext != NULL)
            {
                pEntry->pNext->pPrev = pEntry->pPrev;
            }
            if (pEntry->pPrev != NULL)
            {
                pEntry->pPrev->pNext = pEntry->pNext;
            }
            else
            {
                gl_ResultRoot = pEntry->pNext;
            }
        }
        else
        {
            /* Removed the request from the list */
            if (pEntry->pNext != NULL)
            {
                pEntry->pNext->pPrev = pEntry->pPrev;
            }
            else
            {
                gl_RequestEnd = pEntry->pPrev;
            }
            if (pEntry->pPrev != NULL)
            {
                pEntry->pPrev->pNext = pEntry->pNext;
            }
            else
            {
                gl_RequestRoot = pEntry->pNext;
            }
            if (gl_FIFODepth != 0)
            {
                gl_FIFODepth--;
            }
        }
        Adapter_ThreadMemoryBarrierWrite();
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
        vexLocal_FreeBufferEnty(pEntry);
    }
    else
    {
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
        (void)Adapter_Event_Wait(pEntry->ResultEvent,
                                 vexLocal_ResultEval,
                                 discard_volatile(pEntry),
                                 ADAPTER_VEX_THREAD_SLEEP_MS);

        ret = false;
    }

    return ret;
}

/*----------------------------------------------------------------------------
 * vexLocal_LogicalToken
 *      Execute the logical token request
 */
static VexStatus_t
vexLocal_LogicalToken(VexToken_Command_t *const CommandToken_p,
                      VexToken_Result_t *const ResultToken_p)
{
    VexStatus_t funcres;

    if (vex_LockAcquire() < 0)
    {
        funcres = VEX_LOCK_TIMEOUT;
    }
    else
    {
        if (vex_DeviceGetHandle() != NULL)
        {
            // Device connection is available
            switch (CommandToken_p->OpCode)
            {
            case VEXTOKEN_OPCODE_NOP:
                funcres = vex_Nop(CommandToken_p, ResultToken_p);
                break;

            case VEXTOKEN_OPCODE_ENCRYPTION:
                if (CommandToken_p->SubCode != 0)
                {
                    funcres = vex_SymCipherAE(CommandToken_p, ResultToken_p);
                }
                else
                {
                    funcres = vex_SymCipher(CommandToken_p, ResultToken_p);
                }
                break;

            case VEXTOKEN_OPCODE_HASH:
                funcres = vex_SymHash(CommandToken_p, ResultToken_p);
                break;

            case VEXTOKEN_OPCODE_MAC:
                funcres = vex_SymMac(CommandToken_p, ResultToken_p);
                break;

            case VEXTOKEN_OPCODE_TRNG:
                funcres = vex_Trng(CommandToken_p, ResultToken_p);
                break;

#ifdef VEX_ENABLE_SF
            case VEXTOKEN_OPCODE_SPECIALFUNCTIONS:
                funcres = vex_SpecialFunctions(CommandToken_p, ResultToken_p);
                break;
#endif

            case VEXTOKEN_OPCODE_SYMWRAP:
#ifdef VEX_ENABLE_ENCRYPTED_VECTOR
                if (CommandToken_p->SubCode != 0)
                {
                    funcres = vex_EncryptedVector(CommandToken_p, ResultToken_p);
                }
                else
#endif
                {
                    funcres = vex_SymKeyWrap(CommandToken_p, ResultToken_p);
                }
                break;

            case VEXTOKEN_OPCODE_ASSETMANAGEMENT:
                funcres = vex_Asset(CommandToken_p, ResultToken_p);
                break;

            case VEXTOKEN_OPCODE_AUTH_UNLOCK:
                funcres = vex_AUnlock(CommandToken_p, ResultToken_p);
                break;

            case VEXTOKEN_OPCODE_PUBLIC_KEY:
                funcres = vex_Asym(CommandToken_p, ResultToken_p);
                break;

#ifdef VEX_ENABLE_EMMC
            case VEXTOKEN_OPCODE_EMMC:
                funcres = vex_eMMC(CommandToken_p, ResultToken_p);
                break;
#endif

#ifdef VEX_ENABLE_EXTSERVICE
            case VEXTOKEN_OPCODE_EXTSERVICE:
                funcres = vex_ExtService(CommandToken_p, ResultToken_p);
                break;
#endif

            case VEXTOKEN_OPCODE_SERVICE:
                funcres = vex_Service(CommandToken_p, ResultToken_p);
                break;

            case VEXTOKEN_OPCODE_SYSTEM:
                funcres = vex_System(CommandToken_p, ResultToken_p);
                break;

#ifdef VEX_ENABLE_HW_FUNCTIONS
            case VEXTOKEN_OPCODE_HARDWARE:
                funcres = vex_Hardware(CommandToken_p, ResultToken_p);
                break;
#endif

            case VEXTOKEN_OPCODE_CLAIMCONTROL:
                funcres = vex_Claim(CommandToken_p, ResultToken_p);
                break;

            default:
                funcres = VEX_INVALID_OPCODE;
                break;
            }
        }
        else
        {
            funcres = VEX_NOT_CONNECTED;
        }

        vex_LockRelease();
    }

    return funcres;
}


/*----------------------------------------------------------------------------
 * vexLocal_HandleToken
 *      Thread that monitors the requests in the buffer and executes them one
 *      by one.
 */
static void
vexLocal_HandleToken(void)
{
    volatile struct BufferEntry * pEntry;

    Adapter_ThreadSetCancelState();

    while (!Adapter_ThreadShouldStop())
    {
        Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
        Adapter_ThreadMemoryBarrierRead();
        pEntry = gl_RequestRoot;
        if (pEntry != NULL)
        {
            pEntry->status = EXECUTING;
            Adapter_ThreadMemoryBarrierWrite();
            Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);

            pEntry->ReturnValue = vexLocal_LogicalToken(pEntry->CommandToken_p,
                                                        pEntry->ResultToken_p);

            Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
            pEntry->status = EXECUTED;
            Adapter_ThreadMemoryBarrierWrite();

            gl_RequestRoot = pEntry->pNext;
            if (gl_RequestRoot != NULL)
            {
                gl_RequestRoot->pPrev = NULL;
            }
            else
            {
                gl_RequestEnd = NULL;
            }
            if (gl_FIFODepth != 0)
            {
                gl_FIFODepth--;
            }

            if (gl_ResultRoot != NULL)
            {
                pEntry->pNext = gl_ResultRoot;
                gl_ResultRoot->pPrev = pEntry;
            }
            else
            {
                pEntry->pNext = NULL;
            }
            gl_ResultRoot = pEntry;
            Adapter_ThreadMemoryBarrierWrite();

            /* Notify waiting submitter */
            Adapter_Event_Signal(pEntry->ResultEvent);

            Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);

            Adapter_ThreadYield();
        }
        else
        {
            Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);

            (void)Adapter_Event_Wait(gl_RequestEvent,
                                     vexLocal_RequestEval, NULL,
                                     ADAPTER_VEX_THREAD_SLEEP_MS);
        }
    }
}

/*----------------------------------------------------------------------------
 * vexLocal_KernelToken
 *      Received a kernel based request (token)
 *      - adds it to the processing buffer
 *      - wait for the result
 */
static VexStatus_t
vexLocal_KernelToken(VexToken_Command_t * const CommandToken_p,
                     VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t retValue = VEX_INTERNAL_ERROR;

    /* Make sure token handler thread is available */
    if (gl_HndTokenThread != NULL)
    {
        volatile struct BufferEntry * pEntry;
        unsigned int TimeoutCounter;

        /* Write request to the request list */
        pEntry = vexLocal_RequestWrite(CommandToken_p, ResultToken_p);

        /* Wait for the result */
        TimeoutCounter = gl_FIFODepth * (ADAPTER_VEX_THREAD_REQUEST_TIMEOUT_MS / ADAPTER_VEX_THREAD_SLEEP_MS);
        if (TimeoutCounter == 0)
        {
            TimeoutCounter = (ADAPTER_VEX_THREAD_REQUEST_TIMEOUT_MS / ADAPTER_VEX_THREAD_SLEEP_MS);
        }
        while (pEntry != NULL)
        {
            if (vexLocal_ResultRead(pEntry, (TimeoutCounter == 0), &retValue))
            {
                break;
            }
            else
            {
                Adapter_SleepMS((gl_FIFODepth == 0) ? 1 : gl_FIFODepth);
            }

            TimeoutCounter--;
        }
    }

    return retValue;
}

#ifdef MODULE
static void *
vexLocal_AllocateBuffer(size_t BufferSize,
                        DMAResource_Handle_t * pBufferHandle)
{
    DMAResource_Properties_t Props;
    DMAResource_AddrPair_t AddrPair;
    void * pBuffer = NULL;

    Props.Size = ((BufferSize + 3) & (unsigned int)~3);
    Props.Alignment = 4;

    AddrPair.Address_p = NULL;
    AddrPair.Domain = DMARES_DOMAIN_HOST;

    if (DMAResource_Alloc(Props, &AddrPair, pBufferHandle) < 0)
    {
        // Internal error
        *pBufferHandle = NULL;
    }
    else
    {
        pBuffer = AddrPair.Address_p;
    }
    return pBuffer;
}

static void
vexLocal_ReleaseBuffer(DMAResource_Handle_t * pBufferHandle)
{
    if (*pBufferHandle != NULL)
    {
        DMAResource_Release(*pBufferHandle);
        *pBufferHandle = NULL;
    }
}

static void *
vexLocal_AllocateCopyBuffer(const void * const Data_p,
                            size_t DataSize,
                            DMAResource_Handle_t * pBufferHandle)
{
    void * pBuffer;

    pBuffer = vexLocal_AllocateBuffer((DataSize == 0) ? 4 : DataSize, pBufferHandle);
    if ((pBuffer != NULL) && (DataSize != 0))
    {
        if ((Data_p == NULL) || (copy_from_user(pBuffer, Data_p, DataSize) != 0))
        {
            // Internal error
            vexLocal_ReleaseBuffer(pBufferHandle);
            pBuffer = NULL;
        }
    }
    return pBuffer;
}

static VexStatus_t
vexLocal_CopyReleaseBuffer(VexStatus_t retValue,
                           int  TokenResult,
                           DMAResource_Handle_t * pBufferHandle,
                           void * const pBuffer,
                           void * const Data_p,
                           size_t DataSize)
{
    if ((retValue == VEX_SUCCESS) && (TokenResult >= 0) &&
        (*pBufferHandle != NULL) && (Data_p != NULL) && (DataSize > 0))
    {
        if ((pBuffer == NULL) ||
            (copy_to_user(Data_p, pBuffer, DataSize) != 0))
        {
            retValue = VEX_INTERNAL_ERROR;
        }
    }
    vexLocal_ReleaseBuffer(pBufferHandle);
    return retValue;
}


/*----------------------------------------------------------------------------
 * vexLocal_UserToken
 *      Received a user mode based request (token)
 *      - Allocate and copy input buffer(s)
 *      - Allocate output buffers
 *      - adds it to the processing buffer
 *      - wait for the result
 *      - Release input buffer(s)
 *      - Copy and release output buffers
 */
static VexStatus_t
vexLocal_UserToken(VexToken_Command_t * const CommandToken_p,
                   VexToken_Result_t * const ResultToken_p)
{
    VexToken_Command_t KernelCommand;
    DMAResource_Handle_t BufferHandle[3];
    VexStatus_t retValue = VEX_SUCCESS;

    /* Copy command and result */
    memcpy(&KernelCommand, CommandToken_p, sizeof(VexToken_Command_t));
    KernelCommand.fFromUserSpace = 0;   /* Remove user mode indication */

    /* Convert input and output data buffer to kernel mode
     * - Allocate and copy input buffer(s)
     * - Allocate output buffers */
    BufferHandle[0] = NULL;
    BufferHandle[1] = NULL;
    BufferHandle[2] = NULL;
    switch (CommandToken_p->OpCode)
    {
    case VEXTOKEN_OPCODE_NOP:
        KernelCommand.Service.Nop.In_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.Nop.In_p,
                                                                     CommandToken_p->Service.Nop.Size,
                                                                     &BufferHandle[0]);
        KernelCommand.Service.Nop.Out_p = vexLocal_AllocateBuffer(CommandToken_p->Service.Nop.Size,
                                                                  &BufferHandle[1]);
        if ((KernelCommand.Service.Nop.In_p == NULL) ||
            (KernelCommand.Service.Nop.Out_p == NULL))
        {
            retValue = VEX_INTERNAL_ERROR;
        }
        break;

    case VEXTOKEN_OPCODE_ENCRYPTION:
        if (CommandToken_p->SubCode != 0)
        {
            if (CommandToken_p->Service.CipherAE.AADSize > 0)
            {
                KernelCommand.Service.CipherAE.AAD_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.CipherAE.AAD_p,
                                                                                   CommandToken_p->Service.CipherAE.AADSize,
                                                                                   &BufferHandle[0]);
                if (KernelCommand.Service.CipherAE.AAD_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
            }
            if (CommandToken_p->Service.CipherAE.DataSize > 0)
            {
                KernelCommand.Service.CipherAE.SrcData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.CipherAE.SrcData_p,
                                                                                       CommandToken_p->Service.CipherAE.DataSize,
                                                                                       &BufferHandle[1]);
                if (KernelCommand.Service.CipherAE.SrcData_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
                if (CommandToken_p->Service.CipherAE.DstData_p != NULL)
                {
                    KernelCommand.Service.CipherAE.DstData_p = vexLocal_AllocateBuffer(CommandToken_p->Service.CipherAE.DataSize,
                                                                                       &BufferHandle[2]);
                    if (KernelCommand.Service.CipherAE.DstData_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
            }
        }
        else
        {
            uint32_t DataSize = CommandToken_p->Service.Cipher.DataSize;
            if (DataSize > 0)
            {
                KernelCommand.Service.Cipher.SrcData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.Cipher.SrcData_p,
                                                                                     DataSize,
                                                                                     &BufferHandle[0]);
                if (KernelCommand.Service.Cipher.SrcData_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
            }
            else
            {
                /* Set assumption for output */
                DataSize = (256 / 8);
            }

            KernelCommand.Service.Cipher.DstData_p = vexLocal_AllocateBuffer(DataSize,
                                                                             &BufferHandle[1]);
            if (KernelCommand.Service.Cipher.DstData_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
        break;

    case VEXTOKEN_OPCODE_HASH:
        if (CommandToken_p->Service.Hash.DataSize != 0)
        {
            KernelCommand.Service.Hash.Data_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.Hash.Data_p,
                                                                            CommandToken_p->Service.Hash.DataSize,
                                                                            &BufferHandle[0]);
            if (KernelCommand.Service.Hash.Data_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
        break;

    case VEXTOKEN_OPCODE_MAC:
        if (CommandToken_p->Service.Mac.DataSize != 0)
        {
            KernelCommand.Service.Mac.Data_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.Mac.Data_p,
                                                                           CommandToken_p->Service.Mac.DataSize,
                                                                           &BufferHandle[0]);
            if (KernelCommand.Service.Mac.Data_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
        break;

    case VEXTOKEN_OPCODE_TRNG:
        switch (CommandToken_p->SubCode)
        {
        case VEXTOKEN_SUBCODE_RANDOMNUMBER:
        case VEXTOKEN_SUBCODE_TRNGRAWDATA:
            KernelCommand.Service.Random.Data_p = vexLocal_AllocateBuffer(CommandToken_p->Service.Random.Size,
                                                                          &BufferHandle[0]);
            if (KernelCommand.Service.Random.Data_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
            break;

        case VEXTOKEN_SUBCODE_VERIFYDRBG:
            if (CommandToken_p->Service.DrbgPP.InputData_p != NULL)
            {
                KernelCommand.Service.DrbgPP.InputData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.DrbgPP.InputData_p,
                                                                                       CommandToken_p->Service.DrbgPP.InputDataSize,
                                                                                       &BufferHandle[0]);
                if (KernelCommand.Service.DrbgPP.InputData_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
            }

            KernelCommand.Service.DrbgPP.OutputData_p = vexLocal_AllocateBuffer(CommandToken_p->Service.DrbgPP.OutputDataSize,
                                                                                &BufferHandle[1]);
            if (KernelCommand.Service.DrbgPP.OutputData_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
            break;

        case VEXTOKEN_SUBCODE_VERIFYNRBG:
            KernelCommand.Service.NrbgVerify.InputData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.NrbgVerify.InputData_p,
                                                                                       CommandToken_p->Service.NrbgVerify.Size,
                                                                                       &BufferHandle[0]);
            if (KernelCommand.Service.NrbgVerify.InputData_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
            break;

        default:
            break;
        }
        break;

#ifdef VEX_ENABLE_SF
    case VEXTOKEN_OPCODE_SPECIALFUNCTIONS:
        switch (CommandToken_p->SubCode)
        {
#ifdef VEX_ENABLE_SF_MILENAGE
        case VEXTOKEN_SUBCODE_SF_MILENAGE:
            if (CommandToken_p->Service.SF_Milenage.Operation == VEXTOKEN_MILENAGE_OP_SQNADMINEXPORT)
            {
                KernelCommand.Service.SF_MilenageExport.DataBlob_p = vexLocal_AllocateBuffer(CommandToken_p->Service.SF_MilenageExport.DataBlobSize,
                                                                                             &BufferHandle[0]);
                if (KernelCommand.Service.SF_MilenageExport.DataBlob_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
            }
            break;
#endif

        default:
            break;
        }
        break;
#endif

    case VEXTOKEN_OPCODE_SYMWRAP:
#ifdef VEX_ENABLE_ENCRYPTED_VECTOR
        if (CommandToken_p->SubCode != 0)
        {
            KernelCommand.Service.EncVector.Data_p = vexLocal_AllocateBuffer(CommandToken_p->Service.EncVector.DataSize,
                                                                             &BufferHandle[0]);
            if (KernelCommand.Service.EncVector.Data_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
        else
#endif
        {
            KernelCommand.Service.KeyWrap.SrcData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.KeyWrap.SrcData_p,
                                                                                  CommandToken_p->Service.KeyWrap.SrcDataSize,
                                                                                  &BufferHandle[0]);
            KernelCommand.Service.KeyWrap.DstData_p = vexLocal_AllocateBuffer(CommandToken_p->Service.KeyWrap.DstDataSize,
                                                                              &BufferHandle[1]);
            if ((KernelCommand.Service.KeyWrap.SrcData_p == NULL) ||
                (KernelCommand.Service.KeyWrap.DstData_p == NULL))
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
        break;

    case VEXTOKEN_OPCODE_ASSETMANAGEMENT:
        switch (CommandToken_p->SubCode)
        {
        case VEXTOKEN_SUBCODE_ASSETLOAD:
            switch (CommandToken_p->Service.AssetLoadPlaintext.Method)
            {
            case VEXTOKEN_ASSETLOAD_IMPORT:
                KernelCommand.Service.AssetLoadImport.KeyBlob_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.AssetLoadImport.KeyBlob_p,
                                                                                              CommandToken_p->Service.AssetLoadImport.KeyBlobSize,
                                                                                              &BufferHandle[0]);
                if (KernelCommand.Service.AssetLoadImport.KeyBlob_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
                break;
            case VEXTOKEN_ASSETLOAD_DERIVE:
                if (CommandToken_p->Service.AssetLoadDerive.Salt_p != NULL)
                {
                    KernelCommand.Service.AssetLoadDerive.Salt_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.AssetLoadDerive.Salt_p,
                                                                                               CommandToken_p->Service.AssetLoadDerive.SaltSize,
                                                                                               &BufferHandle[0]);
                    if (KernelCommand.Service.AssetLoadDerive.Salt_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                if (CommandToken_p->Service.AssetLoadDerive.IV_p != NULL)
                {
                    KernelCommand.Service.AssetLoadDerive.IV_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.AssetLoadDerive.IV_p,
                                                                                             CommandToken_p->Service.AssetLoadDerive.IVSize,
                                                                                             &BufferHandle[1]);
                    if (KernelCommand.Service.AssetLoadDerive.IV_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;
            case VEXTOKEN_ASSETLOAD_PLAINTEXT:
                KernelCommand.Service.AssetLoadPlaintext.Data_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.AssetLoadPlaintext.Data_p,
                                                                                              CommandToken_p->Service.AssetLoadPlaintext.DataSize,
                                                                                              &BufferHandle[0]);
                if (KernelCommand.Service.AssetLoadPlaintext.Data_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
                if (CommandToken_p->Service.AssetLoadPlaintext.KekAssetId != 0)
                {
                    KernelCommand.Service.AssetLoadPlaintext.KeyBlob_p = vexLocal_AllocateBuffer(CommandToken_p->Service.AssetLoadPlaintext.KeyBlobSize,
                                                                                                 &BufferHandle[1]);
                    if (KernelCommand.Service.AssetLoadPlaintext.KeyBlob_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;
            case VEXTOKEN_ASSETLOAD_RANDOM:
                if (CommandToken_p->Service.AssetLoadRandom.KekAssetId != 0)
                {
                    KernelCommand.Service.AssetLoadRandom.KeyBlob_p = vexLocal_AllocateBuffer(CommandToken_p->Service.AssetLoadRandom.KeyBlobSize,
                                                                                              &BufferHandle[0]);
                    if (KernelCommand.Service.AssetLoadRandom.KeyBlob_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;
            case VEXTOKEN_ASSETLOAD_SYMUNWRAP:
                KernelCommand.Service.AssetLoadSymUnwrap.KeyBlob_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.AssetLoadSymUnwrap.KeyBlob_p,
                                                                                                 CommandToken_p->Service.AssetLoadSymUnwrap.KeyBlobSize,
                                                                                                 &BufferHandle[0]);
                if (KernelCommand.Service.AssetLoadSymUnwrap.KeyBlob_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
                break;
            default:
                break;
            }
            break;
        case VEXTOKEN_SUBCODE_PUBLICDATA:
            KernelCommand.Service.PublicData.Data_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PublicData.DataSize,
                                                                              &BufferHandle[0]);
            if (KernelCommand.Service.PublicData.Data_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
            break;
        case VEXTOKEN_SUBCODE_MONOTONICREAD:
            KernelCommand.Service.MonotonicCounter.Data_p = vexLocal_AllocateBuffer(CommandToken_p->Service.MonotonicCounter.DataSize,
                                                                                    &BufferHandle[0]);
            if (KernelCommand.Service.MonotonicCounter.Data_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
            break;
        case VEXTOKEN_SUBCODE_OTPDATAWRITE:
            KernelCommand.Service.OTPDataWrite.KeyBlob_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.OTPDataWrite.KeyBlob_p,
                                                                                       CommandToken_p->Service.OTPDataWrite.KeyBlobSize,
                                                                                       &BufferHandle[0]);
            if (KernelCommand.Service.OTPDataWrite.KeyBlob_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
            break;
        case VEXTOKEN_SUBCODE_PROVISIONRANDOMHUK:
            if (CommandToken_p->Service.ProvisionRandomHUK.KeyBlobSize > 0)
            {
                KernelCommand.Service.ProvisionRandomHUK.KeyBlob_p = vexLocal_AllocateBuffer(CommandToken_p->Service.ProvisionRandomHUK.KeyBlobSize,
                                                                                             &BufferHandle[0]);
                if (KernelCommand.Service.ProvisionRandomHUK.KeyBlob_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
            }
            break;
        }
        break;

    case VEXTOKEN_OPCODE_AUTH_UNLOCK:
        if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_AUNLOCKVERIFY)
        {
            KernelCommand.Service.AuthUnlock.Sign_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.AuthUnlock.Sign_p,
                                                                                  CommandToken_p->Service.AuthUnlock.SignSize,
                                                                                  &BufferHandle[0]);
            if (KernelCommand.Service.AuthUnlock.Sign_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
        break;

    case VEXTOKEN_OPCODE_PUBLIC_KEY:
        switch (CommandToken_p->SubCode)
        {
        case VEXTOKEN_SUBCODE_PK_NOASSETS:
            switch (CommandToken_p->Service.PkOperation.Operation)
            {
            case VEXTOKEN_PK_OP_NUMLOAD:
                KernelCommand.Service.PkOperation.InData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkOperation.InData_p,
                                                                                         CommandToken_p->Service.PkOperation.InDataSize,
                                                                                         &BufferHandle[0]);
                if (KernelCommand.Service.PkOperation.InData_p == NULL)
                {
                    retValue = VEX_INTERNAL_ERROR;
                }
                break;

            case VEXTOKEN_PK_OP_MODEXPE:
            case VEXTOKEN_PK_OP_MODEXPD:
            case VEXTOKEN_PK_OP_MODEXPCRT:
            case VEXTOKEN_PK_OP_ECMONTMUL:
            case VEXTOKEN_PK_OP_ECCMUL:
            case VEXTOKEN_PK_OP_ECCADD:
            case VEXTOKEN_PK_OP_DSASIGN:
            case VEXTOKEN_PK_OP_DSAVERIFY:
            case VEXTOKEN_PK_OP_ECDSASIGN:
            case VEXTOKEN_PK_OP_ECDSAVERIFY:
                if (CommandToken_p->Service.PkOperation.InData_p != NULL)
                {
                    KernelCommand.Service.PkOperation.InData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkOperation.InData_p,
                                                                                             CommandToken_p->Service.PkOperation.InDataSize,
                                                                                             &BufferHandle[0]);
                    if (KernelCommand.Service.PkOperation.InData_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                if (CommandToken_p->Service.PkOperation.OutData_p != NULL)
                {
                    KernelCommand.Service.PkOperation.OutData_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkOperation.OutDataSize,
                                                                                          &BufferHandle[1]);
                    if (KernelCommand.Service.PkOperation.OutData_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;

            default:
                break;
            }
            break;

        case VEXTOKEN_SUBCODE_PK_WITHASSETS:
            switch (CommandToken_p->Service.PkAssetSignVerify.Method)
            {
            case VEXTOKEN_PKASSET_ECDH_ECDSA_KEYCHK:
            case VEXTOKEN_PKASSET_DH_DSA_KEYCHK:
                break;

            case VEXTOKEN_PKASSET_DSA_SIGN:
            case VEXTOKEN_PKASSET_ECDSA_SIGN:
            case VEXTOKEN_PKASSET_SM2DSA_SIGN:
            case VEXTOKEN_PKASSET_RSA_PKCS1V1_5_SIGN:
            case VEXTOKEN_PKASSET_RSA_PSS_SIGN:
            case VEXTOKEN_PKASSET_EDDSA_SIGN_INITIAL:
            case VEXTOKEN_PKASSET_EDDSA_SIGN_UPDATE:
            case VEXTOKEN_PKASSET_EDDSA_SIGN_FINAL:
                if (CommandToken_p->Service.PkAssetSignVerify.HashData_p != NULL)
                {
                    KernelCommand.Service.PkAssetSignVerify.HashData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkAssetSignVerify.HashData_p,
                                                                                                     CommandToken_p->Service.PkAssetSignVerify.HashDataSize,
                                                                                                     &BufferHandle[0]);
                    if (KernelCommand.Service.PkAssetSignVerify.HashData_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                if (CommandToken_p->Service.PkAssetSignVerify.Sign_p != NULL)
                {
                    KernelCommand.Service.PkAssetSignVerify.Sign_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkAssetSignVerify.SignSize,
                                                                                             &BufferHandle[1]);
                    if (KernelCommand.Service.PkAssetSignVerify.Sign_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;

            case VEXTOKEN_PKASSET_DSA_VERIFY:
            case VEXTOKEN_PKASSET_ECDSA_VERIFY:
            case VEXTOKEN_PKASSET_SM2DSA_VERIFY:
            case VEXTOKEN_PKASSET_RSA_PKCS1V1_5_VERIFY:
            case VEXTOKEN_PKASSET_RSA_PSS_VERIFY:
            case VEXTOKEN_PKASSET_EDDSA_VERIFY_INITIAL:
            case VEXTOKEN_PKASSET_EDDSA_VERIFY_FINAL:
                if (CommandToken_p->Service.PkAssetSignVerify.HashData_p != NULL)
                {
                    KernelCommand.Service.PkAssetSignVerify.HashData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkAssetSignVerify.HashData_p,
                                                                                                     CommandToken_p->Service.PkAssetSignVerify.HashDataSize,
                                                                                                     &BufferHandle[0]);
                    if (KernelCommand.Service.PkAssetSignVerify.HashData_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                if (CommandToken_p->Service.PkAssetSignVerify.Sign_p != NULL)
                {
                    KernelCommand.Service.PkAssetSignVerify.Sign_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkAssetSignVerify.Sign_p,
                                                                                                 CommandToken_p->Service.PkAssetSignVerify.SignSize,
                                                                                                 &BufferHandle[1]);
                    if (KernelCommand.Service.PkAssetSignVerify.Sign_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;

            case VEXTOKEN_PKASSET_DH_GEN_PUBKEY:
            case VEXTOKEN_PKASSET_ECDH_ECDSA_GEN_PUBKEY:
            case VEXTOKEN_PKASSET_CURVE25519_GEN_PUBKEY:
            case VEXTOKEN_PKASSET_EDDSA_GEN_PUBKEY:
                if (CommandToken_p->Service.PkAssetGenKey.PubKey_p != NULL)
                {
                    KernelCommand.Service.PkAssetGenKey.PubKey_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkAssetGenKey.PubKeySize,
                                                                                           &BufferHandle[0]);
                    if (KernelCommand.Service.PkAssetGenKey.PubKey_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;

            case VEXTOKEN_PKASSET_DH_GEN_KEYPAIR:
            case VEXTOKEN_PKASSET_ECDH_ECDSA_GEN_KEYPAIR:
            case VEXTOKEN_PKASSET_CURVE25519_GEN_KEYPAIR:
            case VEXTOKEN_PKASSET_EDDSA_GEN_KEYPAIR:
                if (CommandToken_p->Service.PkAssetGenKey.KeyBlob_p != NULL)
                {
                    KernelCommand.Service.PkAssetGenKey.KeyBlob_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkAssetGenKey.KeyBlobSize,
                                                                                            &BufferHandle[0]);
                    if (KernelCommand.Service.PkAssetGenKey.KeyBlob_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                if (CommandToken_p->Service.PkAssetGenKey.PubKey_p != NULL)
                {
                    KernelCommand.Service.PkAssetGenKey.PubKey_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkAssetGenKey.PubKeySize,
                                                                                           &BufferHandle[1]);
                    if (KernelCommand.Service.PkAssetGenKey.PubKey_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;

#ifdef VEX_ENABLE_ASYM_ALGO_ECC_ELGAMAL
            case VEXTOKEN_PKASSET_ECC_ELGAMAL_ENCRYPT:
            case VEXTOKEN_PKASSET_ECC_ELGAMAL_DECRYPT:
                if (CommandToken_p->Service.PkAssetEncrypt.SrcData_p != NULL)
                {
                    KernelCommand.Service.PkAssetEncrypt.SrcData_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkAssetEncrypt.SrcData_p,
                                                                                                 CommandToken_p->Service.PkAssetEncrypt.SrcDataSize,
                                                                                                 &BufferHandle[0]);
                    if (KernelCommand.Service.PkAssetEncrypt.SrcData_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                if (CommandToken_p->Service.PkAssetEncrypt.DstData_p != NULL)
                {
                    KernelCommand.Service.PkAssetEncrypt.DstData_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkAssetEncrypt.DstDataSize,
                                                                                             &BufferHandle[1]);
                    if (KernelCommand.Service.PkAssetEncrypt.DstData_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;
#endif

            case VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_SKEYPAIR:
            case VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_DKEYPAIR:
            case VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_SKEYPAIR:
            case VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_DKEYPAIR:
            case VEXTOKEN_PKASSET_CURVE25519_GEN_SHARED_SECRET:
            case VEXTOKEN_PKASSET_SM2KEX_GEN_SHARED_SECRET:
                if ((CommandToken_p->Service.PkAssetGenSharedSecret.OtherInfo_p != NULL) &&
                    (CommandToken_p->Service.PkAssetGenSharedSecret.OtherInfoSize > 0))
                {
                    KernelCommand.Service.PkAssetGenSharedSecret.OtherInfo_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkAssetGenSharedSecret.OtherInfo_p,
                                                                                                           CommandToken_p->Service.PkAssetGenSharedSecret.OtherInfoSize,
                                                                                                           &BufferHandle[0]);
                    if (KernelCommand.Service.PkAssetGenSharedSecret.OtherInfo_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                if ((CommandToken_p->Service.PkAssetGenSharedSecret.Method == VEXTOKEN_PKASSET_SM2KEX_GEN_SHARED_SECRET) &&
                    (CommandToken_p->Service.PkAssetGenSharedSecret.LocalEphPubKey != NULL) &&
                    (CommandToken_p->Service.PkAssetGenSharedSecret.LocalEphPubKeySize > 0))
                {
                    KernelCommand.Service.PkAssetGenSharedSecret.LocalEphPubKey = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkAssetGenSharedSecret.LocalEphPubKey,
                                                                                                              CommandToken_p->Service.PkAssetGenSharedSecret.LocalEphPubKeySize,
                                                                                                              &BufferHandle[1]);
                    if (KernelCommand.Service.PkAssetGenSharedSecret.LocalEphPubKey == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;

            case VEXTOKEN_PKASSET_RSA_OAEP_WRAP_STRING:
            case VEXTOKEN_PKASSET_RSA_OAEP_WRAP_HASH:
            case VEXTOKEN_PKASSET_RSA_OAEP_UNWRAP_STRING:
            case VEXTOKEN_PKASSET_RSA_OAEP_UNWRAP_HASH:
            case VEXTOKEN_PKASSET_RSA_PKCS1V15_WRAP:
            case VEXTOKEN_PKASSET_RSA_PKCS1V15_UNWRAP:
            case VEXTOKEN_PKASSET_SM2ENC_WRAP:
            case VEXTOKEN_PKASSET_SM2ENC_UNWRAP:
                if ((CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_OAEP_WRAP_STRING) ||
                    (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_OAEP_WRAP_HASH) ||
                    (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_PKCS1V15_WRAP) ||
                    (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_SM2ENC_WRAP))
                {
                    KernelCommand.Service.PkAssetWrap.Data_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkAssetWrap.DataSize,
                                                                                       &BufferHandle[0]);
                    if (KernelCommand.Service.PkAssetWrap.Data_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                else
                {
                    KernelCommand.Service.PkAssetWrap.Data_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkAssetWrap.Data_p,
                                                                                           CommandToken_p->Service.PkAssetWrap.DataSize,
                                                                                           &BufferHandle[0]);
                    if (KernelCommand.Service.PkAssetWrap.Data_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;

#ifdef VEX_ENABLE_ASYM_ECIES
            case VEXTOKEN_PKASSET_ECIESWRAP:
            case VEXTOKEN_PKASSET_ECIESUNWRAP:
                if (CommandToken_p->Service.PkEcies.OtherInfo_p != NULL)
                {
                    KernelCommand.Service.PkEcies.OtherInfo_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkEcies.OtherInfo_p,
                                                                                            CommandToken_p->Service.PkEcies.OtherInfoSize,
                                                                                            &BufferHandle[0]);
                    if (KernelCommand.Service.PkEcies.OtherInfo_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }

                if (KernelCommand.Service.PkEcies.Method == VEXTOKEN_PKASSET_ECIESWRAP)
                {
                    KernelCommand.Service.PkEcies.Data_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkEcies.DataSize,
                                                                                   &BufferHandle[1]);
                    if (KernelCommand.Service.PkEcies.Data_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                    KernelCommand.Service.PkEcies.EphPubKey_p = vexLocal_AllocateBuffer(CommandToken_p->Service.PkEcies.EphPubKeySize,
                                                                                        &BufferHandle[2]);
                    if (KernelCommand.Service.PkEcies.EphPubKey_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                else
                {
                    KernelCommand.Service.PkEcies.Data_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.PkEcies.Data_p,
                                                                                       CommandToken_p->Service.PkEcies.DataSize,
                                                                                       &BufferHandle[1]);
                    if (KernelCommand.Service.PkEcies.Data_p == NULL)
                    {
                        retValue = VEX_INTERNAL_ERROR;
                    }
                }
                break;
#endif

            default:
                break;
            }
            break;

        default:
            break;
        }
        break;

#ifdef VEX_ENABLE_EMMC
    case VEXTOKEN_OPCODE_EMMC:
        switch (CommandToken_p->SubCode)
        {
        case VEXTOKEN_SUBCODE_EMMC_RDVER:
        case VEXTOKEN_SUBCODE_EMMC_RDWRCNTVER:
        case VEXTOKEN_SUBCODE_EMMC_WRREQ:
        case VEXTOKEN_SUBCODE_EMMC_WRVER:
            KernelCommand.Service.eMMCRdVerifyWrReqVer.Data_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.eMMCRdVerifyWrReqVer.Data_p,
                                                                                            CommandToken_p->Service.eMMCRdVerifyWrReqVer.DataSize,
                                                                                            &BufferHandle[0]);
            if (KernelCommand.Service.eMMCRdVerifyWrReqVer.Data_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
            break;

        default:
            break;
        }
        break;
#endif

#ifdef VEX_ENABLE_EXTSERVICE
    case VEXTOKEN_OPCODE_EXTSERVICE:
        KernelCommand.Service.ExtService.Data_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.ExtService.Data_p,
                                                                              CommandToken_p->Service.ExtService.DataSize,
                                                                              &BufferHandle[0]);
        if (KernelCommand.Service.ExtService.Data_p == NULL)
        {
            retValue = VEX_INTERNAL_ERROR;
        }
        break;
#endif

#ifdef VEX_ENABLE_FIRMWARE_LOAD
    case VEXTOKEN_OPCODE_SERVICE:
        if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_FIRMWARECHECK)
        {
            KernelCommand.Service.FirmwareCheck.FirmwareImage_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.FirmwareCheck.FirmwareImage_p,
                                                                                              CommandToken_p->Service.FirmwareCheck.FirmwareImageSize,
                                                                                              &BufferHandle[0]);
            if (KernelCommand.Service.FirmwareCheck.FirmwareImage_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
#endif

#ifdef VEX_ENABLE_FIRMWARE_HIBERNATION
    case VEXTOKEN_OPCODE_SYSTEM:
        if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_HIBERNATION)
        {
            KernelCommand.Service.SystemHibernation.DataBlob_p = vexLocal_AllocateBuffer(CommandToken_p->Service.SystemHibernation.DataBlobSize,
                                                                                         &BufferHandle[0]);
            if (KernelCommand.Service.SystemHibernation.DataBlob_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
        else if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_RESUMEFROMHIBERNATION)
        {
            KernelCommand.Service.SystemHibernation.DataBlob_p = vexLocal_AllocateCopyBuffer(CommandToken_p->Service.SystemHibernation.DataBlob_p,
                                                                                             CommandToken_p->Service.SystemHibernation.DataBlobSize,
                                                                                             &BufferHandle[0]);
            if (KernelCommand.Service.SystemHibernation.DataBlob_p == NULL)
            {
                retValue = VEX_INTERNAL_ERROR;
            }
        }
        break;
#endif

    default:
        break;
    }

    if (retValue == VEX_SUCCESS)
    {
        /* Send converted command and result */
        retValue = vexLocal_KernelToken(&KernelCommand, ResultToken_p);

        /* Convert input and output data buffer to use mode
         * - Release input buffer(s)
         * - Copy and release output buffers */
        switch (CommandToken_p->OpCode)
        {
        case VEXTOKEN_OPCODE_NOP:
            vexLocal_ReleaseBuffer(&BufferHandle[0]);
            retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                  ResultToken_p->Result,
                                                  &BufferHandle[1],
                                                  KernelCommand.Service.Nop.Out_p,
                                                  CommandToken_p->Service.Nop.Out_p,
                                                  CommandToken_p->Service.Nop.Size);
            break;

        case VEXTOKEN_OPCODE_ENCRYPTION:
            if (CommandToken_p->SubCode != 0)
            {
                vexLocal_ReleaseBuffer(&BufferHandle[0]);
                if (CommandToken_p->Service.CipherAE.DataSize > 0)
                {
                    vexLocal_ReleaseBuffer(&BufferHandle[1]);
                    if (CommandToken_p->Service.CipherAE.DstData_p != NULL)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[2],
                                                              KernelCommand.Service.CipherAE.DstData_p,
                                                              CommandToken_p->Service.CipherAE.DstData_p,
                                                              CommandToken_p->Service.CipherAE.DataSize);
                    }
                }
            }
            else
            {
                uint32_t DataSize = CommandToken_p->Service.Cipher.DataSize;
                if (DataSize == 0)
                {
                    /* Set assumption for output */
                    DataSize = (256 / 8);
                }

                vexLocal_ReleaseBuffer(&BufferHandle[0]);
                retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                      ResultToken_p->Result,
                                                      &BufferHandle[1],
                                                      KernelCommand.Service.Cipher.DstData_p,
                                                      CommandToken_p->Service.Cipher.DstData_p,
                                                      DataSize);
            }
            break;

        case VEXTOKEN_OPCODE_HASH:
            vexLocal_ReleaseBuffer(&BufferHandle[0]);
            break;

        case VEXTOKEN_OPCODE_MAC:
            vexLocal_ReleaseBuffer(&BufferHandle[0]);
            break;

        case VEXTOKEN_OPCODE_TRNG:
            switch (CommandToken_p->SubCode)
            {
            case VEXTOKEN_SUBCODE_RANDOMNUMBER:
            case VEXTOKEN_SUBCODE_TRNGRAWDATA:
                retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                      ResultToken_p->Result,
                                                      &BufferHandle[0],
                                                      KernelCommand.Service.Random.Data_p,
                                                      CommandToken_p->Service.Random.Data_p,
                                                      CommandToken_p->Service.Random.Size);
                break;

            case VEXTOKEN_SUBCODE_VERIFYDRBG:
                vexLocal_ReleaseBuffer(&BufferHandle[0]);
                retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                      ResultToken_p->Result,
                                                      &BufferHandle[1],
                                                      KernelCommand.Service.DrbgPP.OutputData_p,
                                                      CommandToken_p->Service.DrbgPP.OutputData_p,
                                                      CommandToken_p->Service.DrbgPP.OutputDataSize);
                break;

            case VEXTOKEN_SUBCODE_VERIFYNRBG:
                vexLocal_ReleaseBuffer(&BufferHandle[0]);
                break;

            default:
                break;
            }
            break;

#ifdef VEX_ENABLE_SF
        case VEXTOKEN_OPCODE_SPECIALFUNCTIONS:
            switch (CommandToken_p->SubCode)
            {
#ifdef VEX_ENABLE_SF_MILENAGE
            case VEXTOKEN_SUBCODE_SF_MILENAGE:
                if (CommandToken_p->Service.SF_Milenage.Operation == VEXTOKEN_MILENAGE_OP_SQNADMINEXPORT)
                {
                    retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                          ResultToken_p->Result,
                                                          &BufferHandle[0],
                                                          KernelCommand.Service.SF_MilenageExport.DataBlob_p,
                                                          CommandToken_p->Service.SF_MilenageExport.DataBlob_p,
                                                          CommandToken_p->Service.SF_MilenageExport.DataBlobSize);
                }
                break;
#endif

            default:
                break;
            }
            break;
#endif

        case VEXTOKEN_OPCODE_SYMWRAP:
#ifdef VEX_ENABLE_ENCRYPTED_VECTOR
            if (CommandToken_p->SubCode != 0)
            {
                retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                      ResultToken_p->Result,
                                                      &BufferHandle[0],
                                                      KernelCommand.Service.EncVector.Data_p,
                                                      CommandToken_p->Service.EncVector.Data_p,
                                                      CommandToken_p->Service.EncVector.DataSize);
            }
            else
#endif
            {
                vexLocal_ReleaseBuffer(&BufferHandle[0]);
                retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                      ResultToken_p->Result,
                                                      &BufferHandle[1],
                                                      KernelCommand.Service.KeyWrap.DstData_p,
                                                      CommandToken_p->Service.KeyWrap.DstData_p,
                                                      CommandToken_p->Service.KeyWrap.DstDataSize);
            }
            break;

        case VEXTOKEN_OPCODE_ASSETMANAGEMENT:
            switch (CommandToken_p->SubCode)
            {
            case VEXTOKEN_SUBCODE_ASSETLOAD:
                switch (CommandToken_p->Service.AssetLoadPlaintext.Method)
                {
                case VEXTOKEN_ASSETLOAD_IMPORT:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    break;
                case VEXTOKEN_ASSETLOAD_DERIVE:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    vexLocal_ReleaseBuffer(&BufferHandle[1]);
                    break;
                case VEXTOKEN_ASSETLOAD_PLAINTEXT:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    if (CommandToken_p->Service.AssetLoadPlaintext.KekAssetId)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[1],
                                                              KernelCommand.Service.AssetLoadPlaintext.KeyBlob_p,
                                                              CommandToken_p->Service.AssetLoadPlaintext.KeyBlob_p,
                                                              CommandToken_p->Service.AssetLoadPlaintext.KeyBlobSize);
                    }
                    break;
                case VEXTOKEN_ASSETLOAD_RANDOM:
                    if (CommandToken_p->Service.AssetLoadRandom.KekAssetId)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[0],
                                                              KernelCommand.Service.AssetLoadRandom.KeyBlob_p,
                                                              CommandToken_p->Service.AssetLoadRandom.KeyBlob_p,
                                                              CommandToken_p->Service.AssetLoadRandom.KeyBlobSize);
                    }
                    break;
                case VEXTOKEN_ASSETLOAD_SYMUNWRAP:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    break;
                default:
                    break;
                }
                break;
            case VEXTOKEN_SUBCODE_PUBLICDATA:
                retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                      ResultToken_p->Result,
                                                      &BufferHandle[0],
                                                      discard_const(KernelCommand.Service.PublicData.Data_p),
                                                      discard_const(CommandToken_p->Service.PublicData.Data_p),
                                                      CommandToken_p->Service.PublicData.DataSize);
                break;
            case VEXTOKEN_SUBCODE_MONOTONICREAD:
                retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                      ResultToken_p->Result,
                                                      &BufferHandle[0],
                                                      discard_const(KernelCommand.Service.MonotonicCounter.Data_p),
                                                      discard_const(CommandToken_p->Service.MonotonicCounter.Data_p),
                                                      CommandToken_p->Service.MonotonicCounter.DataSize);
                break;
            case VEXTOKEN_SUBCODE_OTPDATAWRITE:
                vexLocal_ReleaseBuffer(&BufferHandle[0]);
                break;
            case VEXTOKEN_SUBCODE_PROVISIONRANDOMHUK:
                if (CommandToken_p->Service.ProvisionRandomHUK.KeyBlobSize != 0)
                {
                    retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                          ResultToken_p->Result,
                                                          &BufferHandle[0],
                                                          discard_const(KernelCommand.Service.ProvisionRandomHUK.KeyBlob_p),
                                                          discard_const(CommandToken_p->Service.ProvisionRandomHUK.KeyBlob_p),
                                                          CommandToken_p->Service.ProvisionRandomHUK.KeyBlobSize);
                }
                break;
            }
            break;

        case VEXTOKEN_OPCODE_AUTH_UNLOCK:
            vexLocal_ReleaseBuffer(&BufferHandle[0]);
            break;

        case VEXTOKEN_OPCODE_PUBLIC_KEY:
            switch (CommandToken_p->SubCode)
            {
            case VEXTOKEN_SUBCODE_PK_NOASSETS:
                switch (CommandToken_p->Service.PkOperation.Operation)
                {
                case VEXTOKEN_PK_OP_NUMLOAD:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    break;

                case VEXTOKEN_PK_OP_MODEXPE:
                case VEXTOKEN_PK_OP_MODEXPD:
                case VEXTOKEN_PK_OP_MODEXPCRT:
                case VEXTOKEN_PK_OP_ECMONTMUL:
                case VEXTOKEN_PK_OP_ECCMUL:
                case VEXTOKEN_PK_OP_ECCADD:
                case VEXTOKEN_PK_OP_DSASIGN:
                case VEXTOKEN_PK_OP_DSAVERIFY:
                case VEXTOKEN_PK_OP_ECDSASIGN:
                case VEXTOKEN_PK_OP_ECDSAVERIFY:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    if (CommandToken_p->Service.PkOperation.OutData_p != NULL)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[1],
                                                              discard_const(KernelCommand.Service.PkOperation.OutData_p),
                                                              discard_const(CommandToken_p->Service.PkOperation.OutData_p),
                                                              CommandToken_p->Service.PkOperation.OutDataSize);
                    }
                    break;

                default:
                    break;
                }
                break;

            case VEXTOKEN_SUBCODE_PK_WITHASSETS:
                switch (CommandToken_p->Service.PkAssetSignVerify.Method)
                {
                case VEXTOKEN_PKASSET_ECDH_ECDSA_KEYCHK:
                case VEXTOKEN_PKASSET_DH_DSA_KEYCHK:
                    break;

                case VEXTOKEN_PKASSET_DSA_SIGN:
                case VEXTOKEN_PKASSET_ECDSA_SIGN:
                case VEXTOKEN_PKASSET_SM2DSA_SIGN:
                case VEXTOKEN_PKASSET_RSA_PKCS1V1_5_SIGN:
                case VEXTOKEN_PKASSET_RSA_PSS_SIGN:
                case VEXTOKEN_PKASSET_EDDSA_SIGN_INITIAL:
                case VEXTOKEN_PKASSET_EDDSA_SIGN_UPDATE:
                case VEXTOKEN_PKASSET_EDDSA_SIGN_FINAL:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    if (CommandToken_p->Service.PkAssetSignVerify.Sign_p != NULL)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[1],
                                                              KernelCommand.Service.PkAssetSignVerify.Sign_p,
                                                              CommandToken_p->Service.PkAssetSignVerify.Sign_p,
                                                              CommandToken_p->Service.PkAssetSignVerify.SignSize);
                    }
                    break;

                case VEXTOKEN_PKASSET_DSA_VERIFY:
                case VEXTOKEN_PKASSET_ECDSA_VERIFY:
                case VEXTOKEN_PKASSET_SM2DSA_VERIFY:
                case VEXTOKEN_PKASSET_RSA_PKCS1V1_5_VERIFY:
                case VEXTOKEN_PKASSET_RSA_PSS_VERIFY:
                case VEXTOKEN_PKASSET_EDDSA_VERIFY_INITIAL:
                case VEXTOKEN_PKASSET_EDDSA_VERIFY_FINAL:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    vexLocal_ReleaseBuffer(&BufferHandle[1]);
                    break;

                case VEXTOKEN_PKASSET_DH_GEN_PUBKEY:
                case VEXTOKEN_PKASSET_ECDH_ECDSA_GEN_PUBKEY:
                case VEXTOKEN_PKASSET_CURVE25519_GEN_PUBKEY:
                case VEXTOKEN_PKASSET_EDDSA_GEN_PUBKEY:
                    if (CommandToken_p->Service.PkAssetGenKey.PubKey_p != NULL)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[0],
                                                              KernelCommand.Service.PkAssetGenKey.PubKey_p,
                                                              CommandToken_p->Service.PkAssetGenKey.PubKey_p,
                                                              CommandToken_p->Service.PkAssetGenKey.PubKeySize);
                    }
                    break;

                case VEXTOKEN_PKASSET_DH_GEN_KEYPAIR:
                case VEXTOKEN_PKASSET_ECDH_ECDSA_GEN_KEYPAIR:
                case VEXTOKEN_PKASSET_CURVE25519_GEN_KEYPAIR:
                case VEXTOKEN_PKASSET_EDDSA_GEN_KEYPAIR:
                    if (CommandToken_p->Service.PkAssetGenKey.KeyBlob_p != NULL)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[0],
                                                              discard_const(KernelCommand.Service.PkAssetGenKey.KeyBlob_p),
                                                              discard_const(CommandToken_p->Service.PkAssetGenKey.KeyBlob_p),
                                                              CommandToken_p->Service.PkAssetGenKey.KeyBlobSize);
                    }
                    if (CommandToken_p->Service.PkAssetGenKey.PubKey_p != NULL)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[1],
                                                              KernelCommand.Service.PkAssetGenKey.PubKey_p,
                                                              CommandToken_p->Service.PkAssetGenKey.PubKey_p,
                                                              CommandToken_p->Service.PkAssetGenKey.PubKeySize);
                    }
                    break;

#ifdef VEX_ENABLE_ASYM_ALGO_ECC_ELGAMAL
                case VEXTOKEN_PKASSET_ECC_ELGAMAL_ENCRYPT:
                case VEXTOKEN_PKASSET_ECC_ELGAMAL_DECRYPT:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    if (CommandToken_p->Service.PkAssetEncrypt.DstData_p != NULL)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[1],
                                                              discard_const(KernelCommand.Service.PkAssetEncrypt.DstData_p),
                                                              discard_const(CommandToken_p->Service.PkAssetEncrypt.DstData_p),
                                                              CommandToken_p->Service.PkAssetEncrypt.DstDataSize);
                    }
                    break;
#endif

                case VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_SKEYPAIR:
                case VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_DKEYPAIR:
                case VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_SKEYPAIR:
                case VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_DKEYPAIR:
                case VEXTOKEN_PKASSET_CURVE25519_GEN_SHARED_SECRET:
                case VEXTOKEN_PKASSET_SM2KEX_GEN_SHARED_SECRET:
                    vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    vexLocal_ReleaseBuffer(&BufferHandle[1]);
                    break;

                case VEXTOKEN_PKASSET_RSA_OAEP_WRAP_STRING:
                case VEXTOKEN_PKASSET_RSA_OAEP_WRAP_HASH:
                case VEXTOKEN_PKASSET_RSA_OAEP_UNWRAP_STRING:
                case VEXTOKEN_PKASSET_RSA_OAEP_UNWRAP_HASH:
                case VEXTOKEN_PKASSET_RSA_PKCS1V15_WRAP:
                case VEXTOKEN_PKASSET_RSA_PKCS1V15_UNWRAP:
                case VEXTOKEN_PKASSET_SM2ENC_WRAP:
                case VEXTOKEN_PKASSET_SM2ENC_UNWRAP:
                    if ((CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_OAEP_WRAP_STRING) ||
                        (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_OAEP_WRAP_HASH) ||
                        (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_PKCS1V15_WRAP) ||
                        (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_SM2ENC_WRAP))
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[0],
                                                              discard_const(KernelCommand.Service.PkAssetWrap.Data_p),
                                                              discard_const(CommandToken_p->Service.PkAssetWrap.Data_p),
                                                              CommandToken_p->Service.PkAssetWrap.DataSize);
                    }
                    else
                    {
                        vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    }
                    break;

#ifdef VEX_ENABLE_ASYM_ECIES
                case VEXTOKEN_PKASSET_ECIESWRAP:
                case VEXTOKEN_PKASSET_ECIESUNWRAP:
                    if (KernelCommand.Service.PkEcies.OtherInfo_p != NULL)
                    {
                        vexLocal_ReleaseBuffer(&BufferHandle[0]);
                    }

                    if (CommandToken_p->Service.PkEcies.Method == VEXTOKEN_PKASSET_ECIESWRAP)
                    {
                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[1],
                                                              discard_const(KernelCommand.Service.PkEcies.Data_p),
                                                              discard_const(CommandToken_p->Service.PkEcies.Data_p),
                                                              CommandToken_p->Service.PkEcies.DataSize);

                        retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                              ResultToken_p->Result,
                                                              &BufferHandle[2],
                                                              discard_const(KernelCommand.Service.PkEcies.EphPubKey_p),
                                                              discard_const(CommandToken_p->Service.PkEcies.EphPubKey_p),
                                                              CommandToken_p->Service.PkEcies.EphPubKeySize);
                    }
                    else
                    {
                        vexLocal_ReleaseBuffer(&BufferHandle[1]);
                    }
                    break;
#endif

                default:
                    break;
                }
                break;

            default:
                break;
            }
            break;

#ifdef VEX_ENABLE_EMMC
        case VEXTOKEN_OPCODE_EMMC:
            vexLocal_ReleaseBuffer(&BufferHandle[0]);
            break;
#endif

#ifdef VEX_ENABLE_EXTSERVICE
        case VEXTOKEN_OPCODE_EXTSERVICE:
            vexLocal_ReleaseBuffer(&BufferHandle[0]);
            break;
#endif

#ifdef VEX_ENABLE_FIRMWARE_LOAD
        case VEXTOKEN_OPCODE_SERVICE:
            vexLocal_ReleaseBuffer(&BufferHandle[0]);
            break;
#endif

#ifdef VEX_ENABLE_FIRMWARE_HIBERNATION
        case VEXTOKEN_OPCODE_SYSTEM:
            if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_HIBERNATION)
            {
                retValue = vexLocal_CopyReleaseBuffer(retValue,
                                                      ResultToken_p->Result,
                                                      &BufferHandle[0],
                                                      discard_const(KernelCommand.Service.SystemHibernation.DataBlob_p),
                                                      discard_const(CommandToken_p->Service.SystemHibernation.DataBlob_p),
                                                      CommandToken_p->Service.SystemHibernation.DataBlobSize);
            }
            else
            {
                vexLocal_ReleaseBuffer(&BufferHandle[0]);
            }
            break;
#endif

        default:
            break;
        }

        if (retValue == VEX_INTERNAL_ERROR)
        {
            LOG_CRIT(VEX_LOG_PREFIX "%s:%d: Error=%d (%d,%d)\n",
                     __func__, __LINE__, retValue,
                     CommandToken_p->OpCode, CommandToken_p->SubCode);
        }
    }
    else
    {
        LOG_CRIT(VEX_LOG_PREFIX "%s:%d: Error=%d (%d,%d)\n",
                 __func__, __LINE__, retValue,
                 CommandToken_p->OpCode, CommandToken_p->SubCode);

        vexLocal_ReleaseBuffer(&BufferHandle[0]);
        vexLocal_ReleaseBuffer(&BufferHandle[1]);
        vexLocal_ReleaseBuffer(&BufferHandle[2]);
    }

    if (retValue == VEX_INTERNAL_ERROR)
    {
        ResultToken_p->Result = VEX_INTERNAL_ERROR;
    }

    return retValue;
}
#endif

/*----------------------------------------------------------------------------
 * vex_LogicalToken
 *      Receives Requests and add them in the buffer then return results
 */
VexStatus_t
vex_LogicalToken(VexToken_Command_t *const CommandToken_p,
                 VexToken_Result_t *const ResultToken_p)
{
    VexStatus_t retValue = VEX_INTERNAL_ERROR;

#ifdef VEX_STRICT_ARGS
    if ((CommandToken_p == NULL) ||
        (ResultToken_p == NULL))
    {
        retValue = VEX_BAD_ARGUMENT;
    }
    else
#endif

    {
#ifdef MODULE
        if (CommandToken_p->fFromUserSpace)
        {
            retValue = vexLocal_UserToken(CommandToken_p, ResultToken_p);
        }
        else
#endif
        {
            retValue = vexLocal_KernelToken(CommandToken_p, ResultToken_p);
        }
    }

    return retValue;
}


/*----------------------------------------------------------------------------
 * vex_InitLogicalToken
 *      Initialize logical token environment
 */
void vex_InitLogicalToken(void)
{
    int i;

    /* Initialize token buffer environment - pre-allocated free list */
    gl_FreeRoot = &gl_Buffer[0];
    memset(gl_Buffer, 0, sizeof(gl_Buffer));
    for (i = 0; i < (ADAPTER_VEX_BUFFER_PREALLOC_SIZE - 1); i++)
    {
        gl_Buffer[i].pNext = &gl_Buffer[i+1];
    }
    gl_RequestRoot = NULL;
    gl_RequestEnd = NULL;
    gl_ResultRoot = NULL;
    gl_maxFIFODepth = 0;
    gl_nLogicalTokens = 0;

    /* Allocate mutex and event environment */
    gl_AdminMutexlock = Adapter_Lock_Alloc();
    gl_RequestEvent = Adapter_Event_Alloc();

    /* Start thread for sequential token processing */
    gl_HndTokenThread = Adapter_ThreadStart(vexLocal_HandleToken, NULL, "VEX_HndToken");
}

/*----------------------------------------------------------------------------
 * vex_UnInitLogicalToken
 *      De-Initialize logical token environment
 */
void vex_UnInitLogicalToken(void)
{
    volatile struct BufferEntry * pEntry;

    /* Stop token thread */
    if (gl_HndTokenThread != NULL)
    {
        Adapter_ThreadStop(gl_HndTokenThread);
        gl_HndTokenThread = NULL;
    }

    LOG_CRIT(VEX_LOG_PREFIX "Token FIFO: MaxDepth %d, #Tokens %u\n",
             gl_maxFIFODepth, gl_nLogicalTokens);

    /* De-initialize token buffer environment */
    Adapter_Event_Signal(gl_RequestEvent);
    while (gl_RequestRoot != NULL)
    {
        pEntry = gl_RequestRoot;
        gl_RequestRoot = pEntry->pNext;

        vexLocal_FreeBufferEnty(pEntry);
    }
    gl_RequestEnd = NULL;

    while (gl_ResultRoot != NULL)
    {
        pEntry = gl_ResultRoot;
        gl_ResultRoot = pEntry->pNext;

        vexLocal_FreeBufferEnty(pEntry);
    }
    gl_FreeRoot = NULL;

    /* Free mutex and event environment */
    Adapter_Event_Free(gl_RequestEvent);
    gl_RequestEvent = NULL;

    if (gl_AdminMutexlock != NULL)
    {
        Adapter_Lock_Free(gl_AdminMutexlock);
        gl_AdminMutexlock = NULL;
    }

    /* Wait some time to ensure that the thread has been stopped */
    Adapter_SleepMS(50);
}

/* end of file adapter_vex_logicaltoken.c */
