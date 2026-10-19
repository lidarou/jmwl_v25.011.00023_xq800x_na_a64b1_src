/**
 * @file api_val_securetimer.h
 *
 * @brief VAL API - Secure Timer related services
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

#ifndef INCLUDE_GUARD_API_VAL_SECURETIMER_H
#define INCLUDE_GUARD_API_VAL_SECURETIMER_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "api_val_asset.h"          // ValAssetId_t, val_AssetFree


/*-----------------------------------------------------------------------------
 * val_SecureTimerStart
 */
/**
 * This function starts or restarts a Secure Timer. Based on the Second value
 * either a seconds timer or 100 useconds timer created and started.
 * The return or given AssetId is used as Secure Timer reference.
 *
 * @param [in] fSecond
 *     Indication that a seconds timer must be created, otherwise a 100 useconds
 *     timer is created.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId for Secure Timer will
 *     be written when the Timer is successfully started. The AssetId is needed
 *     for all Secure Timer operations.
 *     In case of a restart this pointer points of to the AssetId of an already
 *     created timer
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SecureTimerStart(
        const bool fSecond,
        ValAssetId_t * const AssetId_p);


/*-----------------------------------------------------------------------------
 * val_SecureTimerStop
 */
/**
 * This function stops the Secure Timer after reading the elapsed time.
 * The returned time can be either a number of seconds or 100 useconds
 * depending on the timer.
 * Note that the Secure Timer Asset is released automatically when this
 * function is called, so val_SecureTimerRelease() does not have to be called.
 *
 * @param [in] AssetId
 *     Reference to the Secure Timer Asset.
 *     This reference is provided by val_SecureTimerStart().
 *
 * @param [out] ElapsedTime_p
 *     Pointer to the memory location where the elapsed time must be written.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SecureTimerStop(
        const ValAssetId_t AssetId,
        uint32_t * const ElapsedTime_p);


/*-----------------------------------------------------------------------------
 * val_SecureTimerRead
 */
/**
 * This function reads the elapsed time of Secure Timer. The returned time
 * can be either a number of seconds or 100 useconds depending on the timer.
 *
 * @param [in] AssetId
 *     Reference to the Secure Timer Asset.
 *     This reference is provided by val_SecureTimerStart().
 *
 * @param [out] ElapsedTime_p
 *     Pointer to the memory location where the elapsed time must be written.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SecureTimerRead(
        const ValAssetId_t AssetId,
        uint32_t * const ElapsedTime_p);


/*-----------------------------------------------------------------------------
 * val_SecureTimerRelease
 */
/**
 * This function release the Secure Timer asset.
 * Note that the Secure Timer Asset is released automatically when the function
 * val_SecureTimerStop is called.
 *
 * @param [in] AssetId
 *     Reference to the Secure Timer Asset for releasing the timer.
 *     This reference is provided by val_SecureTimerStart().
 *
 * @return One of the ValStatus_t values.
 */
#define val_SecureTimerRelease(AssetId) val_AssetFree(AssetId)


#endif /* INCLUDE_GUARD_API_VAL_SECURETIMER_H */

/* end of file api_val_securetimer.h */
