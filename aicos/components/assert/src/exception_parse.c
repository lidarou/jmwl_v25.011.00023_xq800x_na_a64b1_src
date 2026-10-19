/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines exception process.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08ac99
#define LOG_MODULE    AMOD_SYS

#include <aic_bitmap.h>
#include <aic_type.h>
#include <assert_config.h>

#include "system_assert.h"

static char exception_reason[128] = {0};

#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)

#define FSR_CODE_BITS   (BIT_0 | BIT_1 | BIT_2 | BIT_10)
#define FSR_SD_BITS     (BIT_12)
#define AFSR_CODE_BITS  (BIT_20 | BIT_22 | BIT_23)

char * exception_reason_parse(u32 fsr, u32 param)
{
    u32 fsr_code, sd;
    u32 afsr_code;

    fsr_code = fsr & FSR_CODE_BITS;
    sd = fsr & FSR_SD_BITS;
    switch (fsr_code) {
    case 0x0:
        strcat(exception_reason, "Background fault!");
        break;
    case 0x1:
        strcat(exception_reason, "Alignment fault!");
        break;
    case 0x2:
        strcat(exception_reason, "Debug event!");
        break;
    case 0x8:
        if(!sd)
            strcat(exception_reason, "Sync ext abort!DECERR or AHB error!");
        else
            strcat(exception_reason, "Sync ext abort!SLVERR");
        break;
    case 0xD:
        strcat(exception_reason, "Permission abort!");
        break;
    case 0x46:
        if(!sd)
            strcat(exception_reason, "Async ext abort!DECERR or AHB error!");
        else
            strcat(exception_reason, "Async ext abort!SLVERR!");
        break;
    case 0x48:
        strcat(exception_reason, "Asyn parity or ECC abort!");
        break;
    case 0x49:
        strcat(exception_reason, "Sync parity or ECC abort!");
        break;
    default:
        strcat(exception_reason, "Unknown abort!");
        break;
    }

    afsr_code = param & AFSR_CODE_BITS;
    switch (afsr_code) {
    case 0x0:
        strcat(exception_reason, "Cache or AXIM!");
        break;
    case 0x100000:
    case 0xC00000:
    case 0xD00000:
        strcat(exception_reason, "Reserved!");
        break;
    case 0x400000:
        strcat(exception_reason, "ATCM!");
        break;
    case 0x500000:
        strcat(exception_reason, "AXI peripheral port!");
        break;
    case 0x800000:
        strcat(exception_reason, "BTCM!");
        break;
    case 0x900000:
        strcat(exception_reason, "AHB peripheral port!");
        break;
    default:
        strcat(exception_reason, "Unknown!");
        break;
    }

    return exception_reason;
}
#elif defined(ASSERT_ARM_ARCH_M)
static char * exception_hard_fault_reason_parse(u32 fsr)
{
    if (fsr & (1 << 1))
        strcat(exception_reason, "Vector table read fault!");
    else if (fsr & (1 << 30))
        strcat(exception_reason, "Debug event occured!");
    else
        strcat(exception_reason, "Reserved!");

    return exception_reason;
}

static char * exception_mem_manage_reason_parse(u32 fsr)
{
    if (fsr & (1 << 0))
        strcat(exception_reason, "MMFSR: Instruction fetch error!");
    else if (fsr & (1 << 1))
        strcat(exception_reason, "MMFSR: Data access error!");
    else if (fsr & (1 << 3))
        strcat(exception_reason, "MMFSR: Exception return error!");
    else if (fsr & (1 << 4))
        strcat(exception_reason, "MMFSR: Exception entry error!");
    else if (fsr & (1 << 5))
        strcat(exception_reason, "MMFSR: FP lazy state preservation error!");
    else
        strcat(exception_reason, "Reserved!");

    return exception_reason;
}

static char * exception_bus_fault_reason_parse(u32 fsr)
{
    if (fsr & (1 << 0))
        strcat(exception_reason, "BFSR: Instruction prefetch error!");
    else if (fsr & (1 << 1))
        strcat(exception_reason, "BFSR: Precise data access error!");
    else if (fsr & (1 << 2))
        strcat(exception_reason, "BFSR: Imprecise data access error!");
    else if (fsr & (1 << 3))
        strcat(exception_reason, "BFSR: Exception return error!");
    else if (fsr & (1 << 4))
        strcat(exception_reason, "BFSR: Exception entry error!");
    else if (fsr & (1 << 5))
        strcat(exception_reason, "BFSR: FP lazy state preservation error!");
    else
        strcat(exception_reason, "Reserved!");

    return exception_reason;
}

static char * exception_usage_fault_reason_parse(u32 fsr)
{
    if (fsr & (1 << 0))
        strcat(exception_reason, "Instruction undefined!");
    else if (fsr & (1 << 1))
        strcat(exception_reason,
                "Instruction executed with invalid T or IT field in EPSR!");
    else if (fsr & (1 << 2))
        strcat(exception_reason, "EXC_RETURN integrity check error!");
    else if (fsr & (1 << 3))
        strcat(exception_reason, "Coprocessor access error!");
    else if (fsr & (1 << 8))
        strcat(exception_reason, "Unaligned access!");
    else if (fsr & (1 << 9))
        strcat(exception_reason, "Divide by zero error!");
    else
        strcat(exception_reason, "Reserved!");

    return exception_reason;
}

char * exception_reason_parse(u32 fsr, u32 param)
{
    char *reason;

    if (param == HARDFAULT_EXCEPTION)
        reason = exception_hard_fault_reason_parse(fsr);
    else if (param == MEMMANAGE_EXCEPTION)
        reason = exception_mem_manage_reason_parse(fsr);
    else if (param == BUSFAULT_EXCEPTION)
        reason = exception_bus_fault_reason_parse(fsr);
    else if (param == USAGEFAULT_EXCEPTION)
        reason = exception_usage_fault_reason_parse(fsr);
    else
        reason = NULL;

    return reason;
}
#else
#error "!!!CPU type not support!!!\n"
#endif
