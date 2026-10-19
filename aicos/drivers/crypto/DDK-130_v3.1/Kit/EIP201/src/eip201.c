/* eip201_sl.c
 *
 * Driver Library for the Security-IP-201 Advanced Interrupt Controller.
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

// Top-level Interrupt controller configuration
#include "c_eip201.h"           // configuration

// Driver Framework Basic Defs API
#include "basic_defs.h"         // uint32_t, inline, etc.

// Interrupt controller API
#include "eip201.h"             // the API we will implement

// Driver Framework Device API
#include "device_rw.h"          // Device_Read32/Write32

// create a constant where all unused interrupts are '1'
#if (EIP201_STRICT_ARGS_MAX_NUM_OF_INTERRUPTS < 32)
#define EIP201_NOTUSEDIRQ_MASK (uint32_t) \
                    (~((1 << EIP201_STRICT_ARGS_MAX_NUM_OF_INTERRUPTS)-1))
#else
#define EIP201_NOTUSEDIRQ_MASK 0
#endif

#ifdef EIP201_STRICT_ARGS
#define EIP201_CHECK_IF_IRQ_SUPPORTED(_irqs) \
        if (_irqs & EIP201_NOTUSEDIRQ_MASK) \
            return EIP201_STATUS_UNSUPPORTED_IRQ;
#else
#define EIP201_CHECK_IF_IRQ_SUPPORTED(_irqs)
#endif /* EIP201_STRICT_ARGS */


/*----------------------------------------------------------------------------
 *  EIP201 registers
 */
enum
{
    EIP201_REGISTER_OFFSET_POL_CTRL     = EIP201_LO_REG_BASE+0,
    EIP201_REGISTER_OFFSET_TYPE_CTRL    = EIP201_LO_REG_BASE+4,
    EIP201_REGISTER_OFFSET_ENABLE_CTRL  = EIP201_LO_REG_BASE+8,
    EIP201_REGISTER_OFFSET_RAW_STAT     = EIP201_HI_REG_BASE+12,
    EIP201_REGISTER_OFFSET_ENABLE_SET   = EIP201_LO_REG_BASE+12,
    EIP201_REGISTER_OFFSET_ENABLED_STAT = EIP201_HI_REG_BASE+16,
    EIP201_REGISTER_OFFSET_ACK          = EIP201_LO_REG_BASE+16,
    EIP201_REGISTER_OFFSET_ENABLE_CLR   = EIP201_LO_REG_BASE+20,
    EIP201_REGISTER_OFFSET_OPTIONS      = EIP201_HI_REG_BASE+24,
    EIP201_REGISTER_OFFSET_VERSION      = EIP201_HI_REG_BASE+28
};

// this implementation supports only the EIP-201 HW1.1 and HW1.2
// 0xC9  = 201
// 0x39  = binary inverse of 0xC9
#define EIP201_SIGNATURE      0x36C9
#define EIP201_SIGNATURE_MASK 0xffff

/*----------------------------------------------------------------------------
 * EIP201_Read32
 *
 * This routine reads from a Register location in the EIP201, applying
 * endianness swapping when required (depending on configuration).
 */
static inline int
EIP201_Read32(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * const Value_p)
{
    return Device_Read32Check(Device, Offset, Value_p);
}


/*----------------------------------------------------------------------------
 * EIP201_Write32
 *
 * This routine writes to a Register location in the EIP201, applying
 * endianness swapping when required (depending on configuration).
 */
static inline int
EIP201_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    return Device_Write32(Device, Offset, Value);
}


/*----------------------------------------------------------------------------
 * EIP201_Config_Change
 */
#ifndef EIP201_REMOVE_CONFIG_CHANGE
EIP201_Status_t
EIP201_Config_Change(
        Device_Handle_t Device,
        const EIP201_SourceBitmap_t Sources,
        const EIP201_Config_t Config)
{
    uint32_t Value;
    uint32_t NewPol = 0;
    uint32_t NewType = 0;
    int rc;
    EIP201_CHECK_IF_IRQ_SUPPORTED(Sources);

    /*
        EIP201_CONFIG_ACTIVE_LOW,       // Type=0, Pol=0
        EIP201_CONFIG_ACTIVE_HIGH,      // Type=0, Pol=1
        EIP201_CONFIG_FALLING_EDGE,     // Type=1, Pol=0
        EIP201_CONFIG_RISING_EDGE       // Type=1, Pol=1
    */

    // do we want Type=1?
    if (Config == EIP201_CONFIG_FALLING_EDGE ||
        Config == EIP201_CONFIG_RISING_EDGE)
    {
        NewType = Sources;
    }

    // do we want Pol=1?
    if (Config == EIP201_CONFIG_ACTIVE_HIGH ||
        Config == EIP201_CONFIG_RISING_EDGE)
    {
        NewPol = Sources;
    }

    if (Sources)
    {
        // modify polarity register
        rc = EIP201_Read32(Device, EIP201_REGISTER_OFFSET_POL_CTRL, &Value);
        if (rc) return rc;
        Value &= ~Sources;
        Value |= NewPol;
        rc = EIP201_Write32(Device, EIP201_REGISTER_OFFSET_POL_CTRL, Value);
        if (rc) return rc;

        // modify type register
        rc = EIP201_Read32(Device, EIP201_REGISTER_OFFSET_TYPE_CTRL, &Value);
        if (rc) return rc;
        Value &= ~Sources;
        Value |= NewType;
        rc = EIP201_Write32(Device, EIP201_REGISTER_OFFSET_TYPE_CTRL, Value);
        if (rc) return rc;
    }

    return EIP201_STATUS_SUCCESS;
}
#endif /* EIP201_REMOVE_CONFIG_CHANGE */


/*----------------------------------------------------------------------------
 * EIP201_Config_Read
 */
#ifndef EIP201_REMOVE_CONFIG_READ

static const EIP201_Config_t EIP201_Setting2Config[4] =
{
    EIP201_CONFIG_ACTIVE_LOW,       // Type=0, Pol=0
    EIP201_CONFIG_ACTIVE_HIGH,      // Type=0, Pol=1
    EIP201_CONFIG_FALLING_EDGE,     // Type=1, Pol=0
    EIP201_CONFIG_RISING_EDGE       // Type=1, Pol=1
};

EIP201_Config_t
EIP201_Config_Read(
        Device_Handle_t Device,
        const EIP201_Source_t Source)
{
    uint32_t Value;
    unsigned char Setting = 0;
    int rc = 0;

    rc = EIP201_Read32(Device, EIP201_REGISTER_OFFSET_TYPE_CTRL, &Value);
    if (rc) return rc;
    if (Value & Source)
    {
        // Type=1, thus edge
        Setting |= 2;
    }

    EIP201_Read32(Device, EIP201_REGISTER_OFFSET_POL_CTRL, &Value);
    if (Value & Source)
    {
        // Pol=1, this rising edge or active high
        Setting |= 1;
    }

    return EIP201_Setting2Config[Setting];
}
#endif /* EIP201_REMOVE_CONFIG_READ */


/*----------------------------------------------------------------------------
 * EIP201_SourceMask_EnableSource
 *
 * See header file for function specifications.
 */
#ifndef EIP201_REMOVE_SOURCEMASK_ENABLESOURCE
EIP201_Status_t
EIP201_SourceMask_EnableSource(
        Device_Handle_t Device,
        const EIP201_SourceBitmap_t Sources)
{
    int rc;
    EIP201_CHECK_IF_IRQ_SUPPORTED(Sources);

    rc = EIP201_Write32(
            Device,
            EIP201_REGISTER_OFFSET_ENABLE_SET,
            Sources);

    return rc;
}
#endif /* EIP201_REMOVE_SOURCEMASK_ENABLESOURCE */


/*----------------------------------------------------------------------------
 * EIP201_SourceMask_DisableSource
 */
#ifndef EIP201_REMOVE_SOURCEMASK_DISABLESOURCE
EIP201_Status_t
EIP201_SourceMask_DisableSource(
        Device_Handle_t Device,
        const EIP201_SourceBitmap_t Sources)
{
    int rc;
    rc = EIP201_Write32(
            Device,
            EIP201_REGISTER_OFFSET_ENABLE_CLR,
            Sources);

    return rc;
}
#endif /* EIP201_REMOVE_SOURCEMASK_DISABLESOURCE */


/*----------------------------------------------------------------------------
 * EIP201_SourceMask_SourceIsEnabled
 */
#ifndef EIP201_REMOVE_SOURCEMASK_SOURCEISENABLED
bool
EIP201_SourceMask_SourceIsEnabled(
        Device_Handle_t Device,
        const EIP201_Source_t Source)
{
    int rc;
    uint32_t SourceMasks;

    rc =  EIP201_Read32(
                        Device,
        EIP201_REGISTER_OFFSET_ENABLE_CTRL, &SourceMasks);

    if (rc) return false;
    if (SourceMasks & Source)
        return true;

    return false;
}
#endif /* EIP201_REMOVE_SOURCEMASK_SOURCEISENABLED */


/*----------------------------------------------------------------------------
 * EIP201_SourceMask_ReadAll
 */
#ifndef EIP201_REMOVE_SOURCEMASK_READALL
EIP201_SourceBitmap_t
EIP201_SourceMask_ReadAll(
        Device_Handle_t Device)
{
    uint32_t Value;
    EIP201_Read32(Device, EIP201_REGISTER_OFFSET_ENABLE_CTRL, &Value);
    return Value;
}
#endif /* EIP201_REMOVE_SOURCEMASK_READALL */


/*----------------------------------------------------------------------------
 * EIP201_SourceStatus_IsEnabledSourcePending
 */
#ifndef EIP201_REMOVE_SOURCESTATUS_ISENABLEDSOURCEPENDING
bool
EIP201_SourceStatus_IsEnabledSourcePending(
        Device_Handle_t Device,
        const EIP201_Source_t Source)
{
    uint32_t Statuses;
    int rc;

    rc = EIP201_Read32(Device, EIP201_REGISTER_OFFSET_ENABLED_STAT, &Statuses);
    if (rc) return false;

    if (Statuses & Source)
        return true;

    return false;
}
#endif /* EIP201_REMOVE_SOURCESTATUS_ISENABLEDSOURCEPENDING */


/*----------------------------------------------------------------------------
 * EIP201_SourceStatus_IsRawSourcePending
 */
#ifndef EIP201_REMOVE_SOURCESTATUS_ISRAWSOURCEPENDING
bool
EIP201_SourceStatus_IsRawSourcePending(
        Device_Handle_t Device,
        const EIP201_Source_t Source)
{
    uint32_t Statuses;
    int rc;

    rc = EIP201_Read32(Device, EIP201_REGISTER_OFFSET_RAW_STAT, &Statuses);
    if (rc) return false;

    if (Statuses & Source)
        return true;

    return false;
}
#endif /* EIP201_REMOVE_SOURCESTATUS_ISRAWSOURCEPENDING */


/*----------------------------------------------------------------------------
 * EIP201_SourceStatus_ReadAllEnabled
 */
#ifndef EIP201_REMOVE_SOURCESTATUS_READALLENABLED
EIP201_SourceBitmap_t
EIP201_SourceStatus_ReadAllEnabled(
        Device_Handle_t Device)
{
    uint32_t Value;
    EIP201_Read32(Device, EIP201_REGISTER_OFFSET_ENABLED_STAT, &Value);
    return Value;
}
#endif /* EIP201_REMOVE_SOURCESTATUS_READALLENABLED */


/*----------------------------------------------------------------------------
 * EIP201_SourceStatus_ReadAllRaw
 */
#ifndef EIP201_REMOVE_SOURCESTATUS_READALLRAW
EIP201_SourceBitmap_t
EIP201_SourceStatus_ReadAllRaw(
        Device_Handle_t Device)
{
    uint32_t Value;
    EIP201_Read32(Device, EIP201_REGISTER_OFFSET_RAW_STAT, &Value);
    return Value;
}
#endif /* EIP201_REMOVE_SOURCESTATUS_READALLRAW */


/*----------------------------------------------------------------------------
 * EIP201_SourceStatus_ReadAllEnabledCheck
 */
#ifndef EIP201_REMOVE_SOURCESTATUS_READALLENABLED
EIP201_Status_t
EIP201_SourceStatus_ReadAllEnabledCheck(
        Device_Handle_t Device,
        EIP201_SourceBitmap_t * const Statuses_p)
{
    return EIP201_Read32(Device, EIP201_REGISTER_OFFSET_ENABLED_STAT, Statuses_p);
}
#endif /* EIP201_REMOVE_SOURCESTATUS_READALLENABLED */


/*----------------------------------------------------------------------------
 * EIP201_SourceStatus_ReadAllRawCheck
 */
#ifndef EIP201_REMOVE_SOURCESTATUS_READALLRAW
EIP201_Status_t
EIP201_SourceStatus_ReadAllRawCheck(
        Device_Handle_t Device,
        EIP201_SourceBitmap_t * const Statuses_p)
{
    return EIP201_Read32(Device, EIP201_REGISTER_OFFSET_RAW_STAT, Statuses_p);
}
#endif /* EIP201_REMOVE_SOURCESTATUS_READALLRAW */


/*----------------------------------------------------------------------------
 * EIP201Lib_Detect
 *
 *  Detect the presence of EIP201 hardware.
 */
#ifndef EIP201_REMOVE_INITIALIZE
static EIP201_Status_t
EIP201Lib_Detect(
        Device_Handle_t Device)
{
    uint32_t Value;
    int rc;

    rc = EIP201_Read32(Device, EIP201_REGISTER_OFFSET_VERSION, &Value);
    if (rc) return rc;
    Value &= EIP201_SIGNATURE_MASK;
    if ( Value != EIP201_SIGNATURE)
        return EIP201_STATUS_UNSUPPORTED_HARDWARE_VERSION;

    // Prevent interrupts going of by disabling them
    rc = EIP201_Write32(Device, EIP201_REGISTER_OFFSET_ENABLE_CTRL, 0);
    if (rc) return rc;

    // Get the number of interrupt sources
    rc = EIP201_Read32(Device, EIP201_REGISTER_OFFSET_OPTIONS, &Value);
    if (rc) return rc;
    // lowest 6 bits contain the number of inputs, which should be between 1-32
    Value &= MASK_6_BITS;
    if (Value == 0 || Value > 32)
        return EIP201_STATUS_UNSUPPORTED_HARDWARE_VERSION;

    return EIP201_STATUS_SUCCESS;
}
#endif /* EIP201_REMOVE_INITIALIZE */


/*----------------------------------------------------------------------------
 * EIP201_Initialize API
 *
 *  See header file for function specification.
 */
#ifndef EIP201_REMOVE_INITIALIZE
EIP201_Status_t
EIP201_Initialize(
        Device_Handle_t Device,
        const EIP201_SourceSettings_t * SettingsArray_p,
        const unsigned int SettingsCount)
{
    EIP201_SourceBitmap_t ActiveLowSources = 0;
    EIP201_SourceBitmap_t ActiveHighSources = 0;
    EIP201_SourceBitmap_t FallingEdgeSources = 0;
    EIP201_SourceBitmap_t RisingEdgeSources = 0;
    EIP201_SourceBitmap_t EnabledSources = 0;
    int rc;

    // check presence of EIP201 hardware
    rc = EIP201Lib_Detect(Device);
    if (rc) return rc;

    // disable all interrupts and set initial configuration
    rc = EIP201_Write32(Device, EIP201_REGISTER_OFFSET_ENABLE_CTRL, 0);
    if (rc) return rc;
    rc = EIP201_Write32(Device, EIP201_REGISTER_OFFSET_POL_CTRL, 0);
    if (rc) return rc;
    rc = EIP201_Write32(Device, EIP201_REGISTER_OFFSET_TYPE_CTRL, 0);
    if (rc) return rc;

    // process the setting, if provided
    if (SettingsArray_p != NULL)
    {
        unsigned int i;

        for (i = 0; i < SettingsCount; i++)
        {
            // check
            const EIP201_Source_t Source = SettingsArray_p[i].Source;
            EIP201_CHECK_IF_IRQ_SUPPORTED(Source);

            // determine polarity
            switch(SettingsArray_p[i].Config)
            {
                case EIP201_CONFIG_ACTIVE_LOW:
                    ActiveLowSources |= Source;
                    break;

                case EIP201_CONFIG_ACTIVE_HIGH:
                    ActiveHighSources |= Source;
                    break;

                case EIP201_CONFIG_FALLING_EDGE:
                    FallingEdgeSources |= Source;
                    break;

                case EIP201_CONFIG_RISING_EDGE:
                    RisingEdgeSources |= Source;
                    break;

                default:
                    // invalid parameter
                    break;
            } // switch

            // determine enabled mask
            if (SettingsArray_p[i].fEnable)
                EnabledSources |= Source;
        } // for
    }

    // program source configuration
    rc = EIP201_Config_Change(
            Device,
            ActiveLowSources,
            EIP201_CONFIG_ACTIVE_LOW);
    if (rc) return rc;

    rc = EIP201_Config_Change(
            Device,
            ActiveHighSources,
            EIP201_CONFIG_ACTIVE_HIGH);
    if (rc) return rc;

    rc = EIP201_Config_Change(
            Device,
            FallingEdgeSources,
            EIP201_CONFIG_FALLING_EDGE);
    if (rc) return rc;

    rc = EIP201_Config_Change(
            Device,
            RisingEdgeSources,
            EIP201_CONFIG_RISING_EDGE);
    if (rc) return rc;

    // the configuration change could have triggered the edge-detection logic
    // so acknowledge all edge-based interrupts immediately
    {
        const uint32_t Value = FallingEdgeSources | RisingEdgeSources;
        rc = EIP201_Write32(Device, EIP201_REGISTER_OFFSET_ACK, Value);
        if (rc) return rc;
    }

    // set mask (enable required interrupts)
    rc = EIP201_SourceMask_EnableSource(Device, EnabledSources);

    return rc;
}
#endif /* EIP201_REMOVE_INITIALIZE */


/*----------------------------------------------------------------------------
 * EIP201_Acknowledge
 *
 * See header file for function specification.
 */
#ifndef EIP201_REMOVE_ACKNOWLEDGE
EIP201_Status_t
EIP201_Acknowledge(
        Device_Handle_t Device,
        const EIP201_SourceBitmap_t Sources)
{
    int rc;
    EIP201_CHECK_IF_IRQ_SUPPORTED(Sources);

    rc = EIP201_Write32(Device, EIP201_REGISTER_OFFSET_ACK, Sources);

    return rc;
}
#endif /* EIP201_REMOVE_ACKNOWLEDGE */

/* end of file eip201_sl.c */
