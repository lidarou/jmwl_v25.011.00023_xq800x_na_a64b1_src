/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines pcdump driver api.
 */

#ifndef __PCDUMP_INC_H__
#define __PCDUMP_INC_H__

struct aic_pcdump_data {
    u32 cr5_pc;
    u32 cr5_r0;
    u32 cr5_r1;
    u32 cr5_r2;
    u32 cr5_r3;
    u32 cr5_r4;
    u32 cr5_r5;
    u32 cr5_r6;
    u32 cr5_r7;
    u32 cr5_usr_r8;
    u32 cr5_usr_r9;
    u32 cr5_usr_r10;
    u32 cr5_usr_r11;
    u32 cr5_usr_r12;
    u32 cr5_usr_r13;
    u32 cr5_usr_r14;
    u32 cr5_irq_r13;
    u32 cr5_irq_r14;
    u32 cr5_svc_r13;
    u32 cr5_svc_r14;
    u32 cr5_abt_r13;
    u32 cr5_abt_r14;
    u32 cr5_und_r13;
    u32 cr5_und_r14;
    u32 cr5_fiq_r8;
    u32 cr5_fiq_r9;
    u32 cr5_fiq_r10;
    u32 cr5_fiq_r11;
    u32 cr5_fiq_r12;
    u32 cr5_fiq_r13;
    u32 cr5_fiq_r14;
    u32 cr5_cpsr_raw;
    u32 cr5_spsr_raw;
};

/*
 * gx_pcdump_get_regs
 * Get CR5 generic registers, include all cpu modes.
 * Notice!!!
 * It should be dumped when CR5 is power on, and CR5 could not
 * be in debug mode, if that the r0\r1 could be wrong.
 */
void gx_pcdump_get_regs(struct aic_pcdump_data *pcdump);

/*
 * gx_pcdump_save_regs
 * Dump CR5 generic registers, include all cpu modes.
 * Notice!!!
 * It should be dumped when CR5 is power on, and CR5 could not
 * be in debug mode, if that the r0\r1 could be wrong.
 */
void gx_pcdump_save_regs(void);

#endif /* __PCDUMP_INC_H__ */

