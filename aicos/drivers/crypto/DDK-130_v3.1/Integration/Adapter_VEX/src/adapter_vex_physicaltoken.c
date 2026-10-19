/* adapter_vex_physicaltoken.c
 *
 * Implementation of the VEX API.
 *
 * This file contains the physical token exchange implementation between the
 * VEX API and EIP-13x hardware.
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

#include "basic_defs.h"             // uint16_t
#include "log.h"

#include "adapter_vex_internal.h"   // API implementation
#include "eip130.h"                 // EIP130_Mailbox*()
#include "adapter_sleep.h"          // Adapter_SleepMS()
#include "adapter_thread.h"         // Adapter_ThreadYield()


#if VEX_USE_INTERRUPTS > 0

#include "adapter_interrupts.h" // Adapter_Interrupts_Init,
#include "adapter_interrupts_ext.h" /* status as well */
#include "adapter_event.h"


/*!
 * Wait set.  Single event just to interact more easily with a Linux waitq.
 */
static Adapter_Event_t gl_MboxEvent = NULL;


/*!
 * Wrapper structure to pass to Adapter_Event_Wait() as arugment to
 * MailboxEval
 */
struct MailboxCondition {
    Device_Handle_t Device;
    uint8_t MailboxNumber;
};

static unsigned gl_MboxIrqCount = 0;
static unsigned gl_MboxCondEvals = 0;


/*----------------------------------------------------------------------------
 * VexLocal_MailboxEval
 */
static int
VexLocal_MailboxEval(void *ctx)
{
    struct MailboxCondition * pCond = ctx;
    ++gl_MboxCondEvals;
    return EIP130_MailboxCanReadToken(pCond->Device, pCond->MailboxNumber);
}


/*----------------------------------------------------------------------------
 * VexLocal_MailboxInterruptHandler
 */
static void
VexLocal_MailboxInterruptHandler(const int IrqNr,
                                 const unsigned Flags)
{
    (void)IrqNr;
    (void)Flags;
    ++gl_MboxIrqCount;
    Adapter_Event_Signal(gl_MboxEvent);    /* all wait on the same flag */
}


/*----------------------------------------------------------------------------
 * vex_InitInterrupts
 */
int
vex_InitInterrupts(void)
{
    int rc = -1;

    if (gl_MboxEvent == NULL)
    {
        gl_MboxEvent = Adapter_Event_Alloc();
    }

    if (gl_MboxEvent != NULL)
    {
        int nMailboxes = vex_DeviceMailboxes();
        int i = 0;

        while (i < nMailboxes)
        {
            if (Adapter_Interrupt_SetHandler(((2 * i) + 1),
                                             VexLocal_MailboxInterruptHandler) < 0)
            {
                break;
            }
            i++;
        }
        if (i < nMailboxes)
        {
            Adapter_Event_Free(gl_MboxEvent);
            gl_MboxEvent = NULL;
        }
        else
        {
            gl_MboxIrqCount = 0;
            gl_MboxCondEvals = 0;
            rc = 0;
        }
    }

    return  rc;
}


/*----------------------------------------------------------------------------
 * vex_UnInitInterrupts
 */
void
vex_UnInitInterrupts(void)
{
    if (gl_MboxEvent != NULL)
    {
        int nMailboxes = vex_DeviceMailboxes();
        int i = 0;

        while (i < nMailboxes)
        {
            (void)Adapter_Interrupt_SetHandler(((2 * i) + 1), NULL);
            i++;
        }

        LOG_CRIT(VEX_LOG_PREFIX "Interrupts: #irqs %u, #evals %u\n",
                 gl_MboxIrqCount, gl_MboxCondEvals);

        Adapter_Event_Free(gl_MboxEvent);
        gl_MboxEvent = NULL;
    }
}
#endif


/*----------------------------------------------------------------------------
 * VexLocal_WaitForResultToken
 */
static int
VexLocal_WaitForResultToken(
        Device_Handle_t Device,
        uint8_t MailboxNumber)
{
    int rc = -1;
#if VEX_USE_INTERRUPTS > 0
    /* Wait on the mailbox event to be set (this is done by the
     * registered interrupt routine. The condition checks if this
     * mailbox is ready (a spurious/stale interrupt reported.).
     */
    int timeout = VEX_POLLING_MAXLOOPS * VEX_POLLING_DELAY_MS;
    struct MailboxCondition cond = { Device, MailboxNumber };

    rc = Adapter_Event_Wait(gl_MboxEvent, VexLocal_MailboxEval, &cond, timeout);
    if (rc > 0)
    {
        LOG_INFO(VEX_LOG_PREFIX "%s: Token received\n", __func__);
        rc = 0;
    }
    else
    {
        /* Force reschedule of process/thread */
        Adapter_ThreadYield();

        if (EIP130_MailboxCanReadToken(Device, MailboxNumber))
        {
            LOG_INFO(VEX_LOG_PREFIX "%s: Token received\n", __func__);
            rc = 0;
        }
        else
        {
            if (rc < 0)
            {
                LOG_CRIT(VEX_LOG_PREFIX "%s: Event error = %d\n", __func__, rc);
            }
            else
            {
                LOG_WARN(VEX_LOG_PREFIX "%s: TOKEN TIMEOUT!\n", __func__);
            }
            rc = -1;
        }
    }
#else
    int SkipSleep = VEX_POLLING_SKIP_FIRST_DELAYS;
    int LoopsLeft = VEX_POLLING_MAXLOOPS;

    // Poll for output token available with sleep
    while (LoopsLeft > 0)
    {
        if (EIP130_MailboxCanReadToken(Device, MailboxNumber))
        {
            // Output token is available!
            LOG_INFO(VEX_LOG_PREFIX "%s: Token received\n", __func__);
            rc = 0;
            break;
        }

        if (SkipSleep > 0)
        {
            // First few rounds are without sleep
            // this avoids sleeping unnecessarily for fast tokens
            SkipSleep--;
        }
        else
        {
            // Sleep a bit
            Adapter_SleepMS(VEX_POLLING_DELAY_MS);
            LoopsLeft--;
            SkipSleep = VEX_POLLING_SKIP_FIRST_DELAYS;
        }
    }

    if (LoopsLeft <= 0)
    {
        // Reached the wait limit
        LOG_WARN(VEX_LOG_PREFIX "%s: TOKEN TIMEOUT!\n", __func__);
    }
#endif
    return rc;
}


/*----------------------------------------------------------------------------
 * vex_PhysicalTokenExchange
 */
VexStatus_t
vex_PhysicalTokenExchange(
        Eip130Token_Command_t * const CommandToken_p,
        Eip130Token_Result_t * const ResultToken_p)
{
    static uint32_t gl_LastSendToken = 0;
    static uint32_t gl_LastHandledToken = 0;
    int funcres;
    int funcres2;
    Device_Handle_t Device;
    uint32_t Identity;
    uint32_t TokenId;
    uint8_t MailboxNumber;

    // Get identity of calling process
    Identity = vex_IdentityGet();
    if (Identity == 0)
    {
        return VEX_NO_IDENTITY;
    }

    // Get mailbox number to use
    MailboxNumber = vex_MailboxGet(Identity);
    if (MailboxNumber == 0)
    {
        return VEX_NO_MAILBOX;
    }

    // Set identity in token if not the Provision Random HUK token
    if ((CommandToken_p->W[0] & (MASK_8_BITS << 24)) !=
        (uint32_t)((EIP130TOKEN_OPCODE_ASSETMANAGEMENT     << 24) |
                   (EIP130TOKEN_SUBCODE_PROVISIONRANDOMHUK << 28)))
    {
        Eip130Token_Command_Identity(CommandToken_p, Identity);
    }

    // Get device reference
    Device = vex_DeviceGetHandle();
    if (Device == NULL)
    {
        return VEX_NOT_CONNECTED;
    }

    // Link the mailbox for use
    funcres = vex_DeviceLinkMailbox(MailboxNumber, Identity);
    if (funcres != 0)
    {
        LOG_CRIT(VEX_LOG_PREFIX "%s: vex_DeviceLinkMailbox()=%d\n", __func__, funcres);
        if (funcres >= -2)
        {
            return VEX_INTERNAL_ERROR;
        }
        return VEX_MAILBOX_IN_USE;
    }

#if (VEX_TRACE_TOKENS > 1)
    LOG_CRIT(VEX_LOG_PREFIX "CommandToken (non-zero words only):\n");
    for (funcres2 = 0; funcres2 < EIP130TOKEN_COMMAND_WORDS; funcres2++)
    {
        if (CommandToken_p->W[funcres2] != 0)
        {
            LOG_CRIT("\tW%02d=0x%08X\n", funcres2, CommandToken_p->W[funcres2]);
        }
    }
#elif (VEX_TRACE_TOKENS > 0)
    LOG_CRIT(VEX_LOG_PREFIX "CommandToken 0x%08X\n", CommandToken_p->W[0]);
#endif

#if VEX_USE_INTERRUPTS > 0
    // Must be enabled before token is submitted, not to loose the interrupt
    Adapter_Interrupt_ClearAndEnable(((2 * (MailboxNumber - 1)) + 1), 0u);
#endif

    // Write the command token to the IN mailbox
    // Note: It also checks that the mailbox is empty
    funcres = EIP130_MailboxWriteAndSubmitToken(Device, MailboxNumber, CommandToken_p, true);
    switch (funcres)
    {
    case 0:
        break;
    case -3:                        /* Token cannot be sent (Mailbox full or HW/FW issue) */
        funcres = VEX_MAILBOX_IN_USE;
        goto func_return;
    case -4:                        /* Handover failed (Is HW active?) */
        vex_DeviceStateIssue();     /* Report device state issue detected */
        funcres = VEX_POWER_STATE_ERROR;
        goto func_return;
    default:
        funcres = VEX_INTERNAL_ERROR;
        goto func_return;
    }
    gl_LastSendToken = CommandToken_p->W[0];

    // Loop to handle a possible previous timeout situation
    TokenId = CommandToken_p->W[0] & MASK_16_BITS;
    ResultToken_p->W[0] = 0;
    do
    {
        if (ResultToken_p->W[0] != 0)
        {
            LOG_CRIT(VEX_LOG_PREFIX "Dropped ResultToken 0x%08X\n", ResultToken_p->W[0]);
            ResultToken_p->W[0] = 0;
        }

        // Wait for the result token to be available
        if (VexLocal_WaitForResultToken(Device, MailboxNumber) != 0)
        {
            funcres = VEX_REPONSE_TIMEOUT;
            goto func_return;
        }

        // Read the result token from the OUT mailbox
        if (EIP130_MailboxReadToken(Device, MailboxNumber, ResultToken_p) != 0)
        {
            funcres = VEX_INTERNAL_ERROR;
            goto func_return;
        }
    } while (TokenId != (ResultToken_p->W[0] & MASK_16_BITS));
    gl_LastHandledToken = CommandToken_p->W[0];

#if (VEX_TRACE_TOKENS_ERROR > 0)
    if (ResultToken_p->W[0] & BIT_31)
    {
#if (VEX_TRACE_TOKENS_ERROR > 1)
        LOG_CRIT(VEX_LOG_PREFIX "ERROR: ResultToken 0x%08X for CommandToken (non-zero words only):\n",
                 ResultToken_p->W[0]);
        for (funcres2 = 0; funcres2 < EIP130TOKEN_COMMAND_WORDS; funcres2++)
        {
            if (CommandToken_p->W[funcres2] != 0)
            {
                LOG_CRIT("\tW%02d=0x%08X\n", funcres2, CommandToken_p->W[funcres2]);
            }
        }
#else
        LOG_CRIT(VEX_LOG_PREFIX "ERROR: ResultToken 0x%08X for CommandToken 0x%08X\n",
                 ResultToken_p->W[0], CommandToken_p->W[0]);
#endif
    }
#endif
#if (VEX_TRACE_TOKENS > 1)
    LOG_CRIT(VEX_LOG_PREFIX "ResultToken (non-zero words only):\n");
    for (funcres2 = 0; funcres2 < EIP130TOKEN_RESULT_WORDS; funcres2++)
    {
        if (ResultToken_p->W[funcres2] != 0)
        {
            LOG_CRIT("\tW%02d=0x%08X\n", funcres2, ResultToken_p->W[funcres2]);
        }
    }
#elif (VEX_TRACE_TOKENS > 0)
    LOG_CRIT(VEX_LOG_PREFIX "ResultToken 0x%08X\n", ResultToken_p->W[0]);
#endif

func_return:
    if (funcres != 0)
    {
        LOG_CRIT(VEX_LOG_PREFIX "%s: Error=%d (CommandToken 0x%08X <= 0x%08X,0x%08X)\n",
                 __func__, funcres, CommandToken_p->W[0], gl_LastSendToken, gl_LastHandledToken);
    }
    funcres2 = vex_DeviceUnlinkMailbox(MailboxNumber, Identity);
    if (funcres2 != 0)
    {
        LOG_CRIT(VEX_LOG_PREFIX "%s: vex_DeviceUnlinkMailbox()=%d\n",
                 __func__, funcres2);
    }

    return funcres;
}


/* end of file adapter_vex_physicaltoken.c */
