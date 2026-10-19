/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert related infomations.
 */

#ifndef __SYSTEM_ASSERT_H__
#define __SYSTEM_ASSERT_H__

#include <aic_type.h>
#include <assert_config.h>

#define ASSERT_MAX_COUNT    2

#define SPACE_CHAR          ' '
#define SPACE_NUM           6

#define FIQ_MODE            0x11
#define IRQ_MODE            0x12
#define SVC_MODE            0x13

enum assert_ar_type {
    AR_NORMAL_ASSERT = 0,
    AR_EXCEPTION_ASSERT,
    AR_IRQ_ASSERT,
    AR_FIQ_ASSERT
};

enum assert_ar_exception_type {
    UND_EXCEPTION = 1,
    DABT_EXCEPTION,
    PABT_EXCEPTION,
    SWI_EXCEPTION,
};

enum assert_m_type {
    M_NORMAL_ASSERT = 0,
    M_EXCEPTION_ASSERT,
    M_IRQ_ASSERT,
};

enum assert_m_exception_type {
    HARDFAULT_EXCEPTION = 1,
    MEMMANAGE_EXCEPTION,
    BUSFAULT_EXCEPTION,
    USAGEFAULT_EXCEPTION
};

enum assert_reason {
    ACTIVE_ASSERT = 0,
    PASSIVE_ASSERT
};

enum assert_mode {
    DEBUG_ASSERT = 0,
    RELEASE_ASSERT = 0x52454C52
};

struct assert_ar_common_reg {
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 r12;
    u32 r13;
    u32 r14;
    u32 pc;
    u32 spsr;
    u32 cpsr;
};

struct assert_ar_fiq_bank_reg {
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 r12;
    u32 r13;
    u32 r14;
    u32 spsr;
};

struct assert_ar_bank_reg {
    u32 r13;
    u32 r14;
    u32 spsr;
};

struct assert_ar_fault_status_reg {
    u32 far;
    u32 fsr;
    u32 afsr;
};

struct assert_ar_regs {
    struct assert_ar_common_reg cur_mode_reg;
    struct assert_ar_bank_reg svc_mode_reg;
    struct assert_ar_bank_reg irq_mode_reg;
    struct assert_ar_bank_reg und_mode_reg;
    struct assert_ar_bank_reg abt_mode_reg;
    struct assert_ar_fiq_bank_reg fiq_mode_reg;
    struct assert_ar_fault_status_reg dabt_reg;
    struct assert_ar_fault_status_reg pabt_reg;
};

struct assert_m_common_reg {
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 r12;
    u32 r13;
    u32 r14;
    u32 pc;
    u32 xpsr;
    u32 msp;
    u32 psp;
    u32 control;
};

struct assert_m_exception_frame {
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r12;
    u32 lr;
    u32 pc;
    u32 xpsr;
};

struct assert_m_fault_status_reg {
    u32 fault_status;
    u32 fault_address;
};

struct assert_m_regs {
    struct assert_m_common_reg cur_mode_reg;
    struct assert_m_exception_frame exc_frame;
    struct assert_m_fault_status_reg fault_reg;
    u32 exc_return;
};

struct assert_common_info {
    u32 assert_type;
    u32 exception_type;
    enum assert_reason reason;
    u32 assert_step;
    const char *info_string;
#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)
    struct assert_ar_regs regs;
#elif defined(ASSERT_ARM_ARCH_M)
    struct assert_m_regs regs;
#else
#error "!!!CPU type not support!!!\n"
#endif
};

struct assert_info {
    u32 assert_cnt;
    char *version;
    boolean q_ful_assert;
    boolean assert_status;
    boolean assert_notified;
    enum assert_mode mode;

    struct assert_common_info info[ASSERT_MAX_COUNT];
};

#endif /* __SYSTEM_ASSERT_H__ */

