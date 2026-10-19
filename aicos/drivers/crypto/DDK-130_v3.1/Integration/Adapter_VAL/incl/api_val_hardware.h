/**
 * @file api_val_hardware.h
 *
 * @brief VAL API - Hardware related services
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

#ifndef INCLUDE_GUARD_API_VAL_HARDWARE_H
#define INCLUDE_GUARD_API_VAL_HARDWARE_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"             // uint8_t, uint16_t, uint32_t, uint64_t

#include "api_val_result.h"         // ValStatus_t


/*-----------------------------------------------------------------------------
 * val_HW_ModuleStatus
 */
/**
 * This function retrieves the module status from the EIP-13x hardware
 * registers and returns the requested/selected information.
 *
 * @param [out] FIPSmode_p
 *     Pointer to the variable in which FIPS mode indication must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (not active) or 1 (active).
 *
 * @param [out] NonFIPSmode_p
 *     Pointer to the variable in which non-FIPS mode indication must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (not active) or 1 (active).
 *
 * @param [out] FatalError_p
 *     Pointer to the variable in which the fatal error indication must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (no error) or 1 (error).
 *
 * @param [out] CRC24Ok_p
 *     Pointer to the variable in which the CRC24 Ok indication must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (not Ok) or 1 (Ok).
 *
 * @param [out] CRC24Busy_p
 *     Pointer to the variable in which the CRC24 busy indication must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (not busy) or 1 (busy).
 *
 * @param [out] CRC24Error_p
 *     Pointer to the variable in which the CRC24 error indication must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (no error) or 1 (error).
 *
 * @param [out] FwImageWritten_p
 *     Pointer to the variable in which the Firmware image written indication
 *     must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (not written) or 1 (written).
 *
 * @param [out] FwImageCheckDone_p
 *     Pointer to the variable in which the Firmware image check done
 *     indication must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (not checked/check busy) or 1 (check done).
 *
 * @param [out] FwImageAccepted_p
 *     Pointer to the variable in which the Firmware image accepted indication
 *     must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 (not accepted) or 1 (accepted).
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_HW_ModuleStatus(
        uint8_t * const FIPSmode_p,
        uint8_t * const NonFIPSmode_p,
        uint8_t * const FatalError_p,
        uint8_t * const CRC24Ok_p,
        uint8_t * const CRC24Busy_p,
        uint8_t * const CRC24Error_p,
        uint8_t * const FwImageWritten_p,
        uint8_t * const FwImageCheckDone_p,
        uint8_t * const FwImageAccepted_p);


/*-----------------------------------------------------------------------------
 * val_HW_EIP_Options
 */
/**
 * This function retrieves the EIP options information from the EIP-13x
 * hardware registers and returns the requested/selected information.
 *
 * @param [out] nMailboxes_p
 *     Pointer to the variable in which the number of mailboxes must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be in the range [1..8].
 *
 * @param [out] MailboxSize_p
 *     Pointer to the variable in which the mailbox size must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 128, 256 (default), 512 and 1024 bytes.
 *
 * @param [out] HostId_p
 *     Pointer to the variable in which the Host Id must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *
 * @param [out] SecureHostId_p
 *     Pointer to the variable in which the Secure Host Id must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *
 * @param [out] MasterId_p
 *     Pointer to the variable in which the master host Id must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be in the range [0..7].
 *
 * @param [out] MyHostId_p
 *     Pointer to the variable in which the my host Id (calling host) must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be in the range [0..7].
 *
 * @param [out] ProtectionAvailable_p
 *     Pointer to the variable in which the protection available indication
 *     must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 or 1.
 *
 * @param [out] Protection_p
 *     Pointer to the variable in which the protection indication must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *     The value can be 0 or 1.
 *
 * @param [out] StandardEngines_p
 *     Pointer to the variable in which standard engines, cpu and bus interface
 *     information must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *
 * @param [out] CustomEngines_p
 *     Pointer to the variable in which custom engines information must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_HW_EIP_Options(
        uint8_t * const nMailboxes_p,
        uint16_t * const MailboxSize_p,
        uint8_t * const HostId_p,
        uint8_t * const SecureHostId_p,
        uint8_t * const MasterId_p,
        uint8_t * const MyHostId_p,
        uint8_t * const ProtectionAvailable_p,
        uint8_t * const Protection_p,
        uint16_t * const StandardEngines_p,
        uint16_t * const CustomEngines_p);


/*-----------------------------------------------------------------------------
 * val_HW_EIP_Version
 */
/**
 * This function retrieves the EIP version and number information from the
 * EIP-13x hardware registers and returns the requested/selected information.
 *
 * @param [out] MajorVersion_p
 *     Pointer to the variable in which the major version number of the
 *     hardware must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *
 * @param [out] MinorVersion_p
 *     Pointer to the variable in which the minor version number of the
 *     hardware must be returned.
 *     Note: Pointer may be NULL if information is not required.
 *
 * @param [out] PatchLevel_p
 *     Pointer to the variable in which patch level of the hardware must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *
 * @param [out] EipNumber_p
 *     Pointer to the variable in which EIP number of the hardware must be
 *     returned.
 *     Note: Pointer may be NULL if information is not required.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_HW_EIP_Version(
        uint8_t * const MajorVersion_p,
        uint8_t * const MinorVersion_p,
        uint8_t * const PatchLevel_p,
        uint8_t * const EipNumber_p);


#endif /* INCLUDE_GUARD_API_VAL_HARDWARE_H */

/* end of file api_val_hardware.h */
