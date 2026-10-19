/* adapter_vex_identity.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the identity related functionality.
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

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "adapter_getpid.h"
#include "adapter_vex_internal.h"   // API implementation

#define VEX_IDENTITY_MAX_USERS 4
#define VEX_IDENTITY_CO_INDEX  VEX_IDENTITY_MAX_USERS

struct vex_identity
{
   int      pid;
   uint32_t identity;
};

static struct vex_identity gl_Identities[VEX_IDENTITY_MAX_USERS + 1] =
{
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, VEX_CRYPTO_OFFICER_ID }
};


/*----------------------------------------------------------------------------
 * vex_IdentityGet
 */
uint32_t
vex_IdentityGet(void)
{
    int pid = Adapter_GetPid();
    uint32_t identity = 0;
    int i;

    for (i = 0; i < (VEX_IDENTITY_MAX_USERS + 1); i++)
    {
        if (gl_Identities[i].pid == pid)
        {
            //identity = gl_Identities[i].identity;
            break;
        }
    }
    if (identity == 0)
    {
        // Add process if not known
        //identity = vex_IdentityUserAdd();
    }

    if (vex_DeviceIsSecureConnected())
    {
        identity = VEX_CRYPTO_OFFICER_ID;
    }
    else
    {
        identity = (uint32_t)(VEX_CRYPTO_OFFICER_ID ^ -1);
    }
    return identity;
}

/*----------------------------------------------------------------------------
 * vex_IdentityCryptoOfficer
 */
void
vex_IdentityCryptoOfficer(
        uint32_t CryptoOfficerId)
{
    gl_Identities[VEX_IDENTITY_CO_INDEX].pid = Adapter_GetPid();
    if (CryptoOfficerId != 0)
    {
        gl_Identities[VEX_IDENTITY_CO_INDEX].identity = CryptoOfficerId;
    }
    else
    {
        gl_Identities[VEX_IDENTITY_CO_INDEX].identity = VEX_CRYPTO_OFFICER_ID;
    }
    LOG_CRIT(VEX_LOG_PREFIX "Set CryptoOfficerId: Process ID=0x%x (%u)\n",
             gl_Identities[VEX_IDENTITY_CO_INDEX].pid,
             gl_Identities[VEX_IDENTITY_CO_INDEX].identity);
}


/*----------------------------------------------------------------------------
 * vex_IdentityUserAdd
 */
uint32_t
vex_IdentityUserAdd(void)
{
    static uint32_t gl_IdentityRef = 0x1;
    int pid = Adapter_GetPid();
    uint32_t identity = 0;
    unsigned int i;

    for (i = 0; i < VEX_IDENTITY_MAX_USERS; i++)
    {
        if (gl_Identities[i].pid == pid)
        {
            identity = gl_Identities[i].identity;
            break;
        }
    }
    if (identity == 0)
    {
        // Add process if not known
        // Note: Setting of Crypto Officer pid requires
        //       vex_IdentityCryptoOfficer()
        gl_IdentityRef++;
        gl_IdentityRef &= 0x0FFFFFFF;
        if (gl_IdentityRef == 0)
        {
            gl_IdentityRef = 0x1;
        }
        for (i = 0; i < VEX_IDENTITY_MAX_USERS; i++)
        {
            if (gl_Identities[i].pid == 0)
            {
                gl_Identities[i].pid = pid;
                identity = (uint32_t)((gl_IdentityRef << 4) + i);
                gl_Identities[i].identity = identity;

                LOG_CRIT(VEX_LOG_PREFIX "Add User%d: Process ID=0x%x (%u)\n",
                         i+1,
                         gl_Identities[i].pid,
                         gl_Identities[i].identity);
                break;
            }
        }
        if (identity == 0)
        {
            LOG_CRIT(VEX_LOG_PREFIX "FAILED to add User - No free entry\n");
        }
    }

    return identity;
}


/*----------------------------------------------------------------------------
 * vex_IdentityUserRemove
 */
void
vex_IdentityUserRemove(void)
{
    int pid = Adapter_GetPid();
    int i;

    for (i = 0; i < VEX_IDENTITY_MAX_USERS; i++)
    {
        if (gl_Identities[i].pid == pid)
        {
            gl_Identities[i].pid = 0;
            gl_Identities[i].identity = 0;

            LOG_CRIT(VEX_LOG_PREFIX "Remove User%d: Process ID=0x%x\n",
                     i+1, pid);
            break;
        }
    }
}

/* end of file adapter_vex_identity.c */
