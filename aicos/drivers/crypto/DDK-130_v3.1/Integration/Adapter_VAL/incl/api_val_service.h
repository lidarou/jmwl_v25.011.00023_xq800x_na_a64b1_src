/**
 * @file api_val_service.h
 *
 * @brief VAL API - Service related services
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

#ifndef INCLUDE_GUARD_API_VAL_SERVICE_H
#define INCLUDE_GUARD_API_VAL_SERVICE_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"                 // uint8_t, uint16_t, uint32_t, uint64_t

#include "api_val_result.h"             // ValStatus_t


/*-----------------------------------------------------------------------------
 * val_ServiceRegisterRead
 */
/**
 * This function reads the specified internal register (address) of EIP-13x
 * hardware.
 *
 * @param [in] Address
 *     Address of the internal register to write.
 *
 * @param [out] Value_p
 *     Pointer to the buffer in which the value needs to be returned.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ServiceRegisterRead(
        uint32_t         Address,
        uint32_t * const Value_p);


/*-----------------------------------------------------------------------------
 * val_ServiceRegisterWrite
 */
/**
 * This function writes a value to the specified internal register (address)
 * of EIP-13x hardware.
 *
 * @param [in] Address
 *     Address of the internal register to write.
 *
 * @param [in] Value
 *     Value to write.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ServiceRegisterWrite(
        uint32_t Address,
        uint32_t Value);


/*-----------------------------------------------------------------------------
 * val_ServiceZeroizeMailbox
 */
/**
 * This function zeroizes the output mailbox to make sure that the output
 * mailbox does not hold sensitive information when the mailbox is released.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ServiceZeroizeMailbox(void);


/*-----------------------------------------------------------------------------
 * val_ServiceSelectOTPZeroize
 */
/**
 * This function selects the OTP zeroize functionality only. To execute the
 * actual OTP zeroize operation, this function call must be followed by calling
 * the function val_ServiceZeroizeOTP().
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ServiceSelectOTPZeroize(void);


/*-----------------------------------------------------------------------------
 * val_ServiceZeroizeOTP
 */
/**
 * This function executes the OTP zeroize operation, when the OTP zeroize
 * functionality was previously selected with the val_ServiceSelectOTPZeroize()
 * function.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ServiceZeroizeOTP(void);


/*-----------------------------------------------------------------------------
 * val_ServiceClockSwitch
 */
/**
 * This function controls the switch that activates the Encryption, Hash, TRNG,
 * PKCP, Sequencer and Custom Engine clocks. Normally, the EIP-13x firmware
 * switches the clocks automatically for optimized power consumption.
 * Notes:
 * - Function is only intended for debug
 * - All clocks with 0b's in the ClocksForcedOn and ClocksForcedOff bitmask are
 *   set to 'Automatic mode'.
 *
 * @param [in] ClocksForcedOn
 *     Bitmask specifying (with 1b's) the clocks that needs to be forced ON
 *     (always ON).
 *
 * @param [in] ClocksForcedOff
 *     Bitmask specifying (with 1b's) the clocks that needs to be forced OFF
 *     (always OFF).
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ServiceClockSwitch(
        uint16_t ClocksForcedOn,
        uint16_t ClocksForcedOff);


/*-----------------------------------------------------------------------------
 * val_ServiceFirmwareCheck
 */
/**
 * The Firmware Check function verifies that a firmware image is valid and can
 * be used in the RAM-based Firmware Code Validation and Boot (System Boot)
 * process. The operation * is successful when the image is not corrupted,
 * and the RollbackID value is accepted (e.g. higher than or equal to the
 * current image RollbackID value).
 *
 * @param [in] FWImage_p
 *      Pointer to the firmware image.
 *
 * @param [in] FWImageSize
 *      Size of the firmware image in bytes.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ServiceFirmwareCheck(
        const uint8_t * const FWImage_p,
        const uint32_t FWImageSize);


/*-----------------------------------------------------------------------------
 * val_ServiceUpdateRollbackID
 */
/**
 * Update Rollback ID operation updates the SFWBVER monotonic counter (RollbackID)
 * with either the value of the current firmware or the specified value.
 *
 * @param [in] RollbackID
 *      The new value for the SFWBVER. zero to update it to the RollbackID
 *      of the current firmware.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ServiceUpdateRollbackID(
        uint16_t RollbackID);


#endif /* INCLUDE_GUARD_API_VAL_SERVICE_H */

/* end of file api_val_service.h */
