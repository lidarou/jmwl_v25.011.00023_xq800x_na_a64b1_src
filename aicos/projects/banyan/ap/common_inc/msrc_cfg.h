/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    msrc cali init parameters.
*/

#ifndef __MSRC_CFG_H__
#define __MSRC_CFG_H__

#include <irq_cfg.h>

/* cali interval, uint is ms */
#define MSRC0_CALI_INTERVAL        300000
#define MSRC1_CALI_INTERVAL        60000
#define RC32K_CALI_INTERVAL        60000

#define MSRC_CALI_IRQ              IRQ_TOP_RCCALI

struct msrc_cali_cfg {
    u32 cali_golden_value;
    u32 cali_cycle;
    u16 cali_margin_cnt;
    u16 cali_stable_num;
    u8  cali_step;
    u8  cali_timeout_val;
    boolean cali_auto_mode;
};

struct msrc_cali_cfg g_msrc0_ab_cfg = {
    .cali_golden_value = 376157,
    .cali_cycle = 1000,
    .cali_margin_cnt = 400,
    .cali_stable_num = 30,
    .cali_step = 7,
    .cali_timeout_val = 255,
    .cali_auto_mode = TRUE,
};

struct msrc_cali_cfg g_mc_cfg[3] = {
    /* msrc1 */
    {
        .cali_golden_value = 54167,
        .cali_cycle = 10000,
        .cali_margin_cnt = 60,
        .cali_stable_num = 200,
        .cali_step = 4,
        .cali_timeout_val = 255,
        .cali_auto_mode = TRUE,
    },
    /* msrc0 */
    {
        .cali_golden_value = 112847,
        .cali_cycle = 300,
        .cali_margin_cnt = 120,
        .cali_stable_num = 30,
        .cali_step = 5,
        .cali_timeout_val = 255,
        .cali_auto_mode = TRUE,
    },

    /* rc32k */
    {
        .cali_golden_value = 793457,
        .cali_cycle = 100,
        .cali_margin_cnt = 50,
        .cali_stable_num = 3,
        .cali_step = 5,
        .cali_timeout_val = 255,
        .cali_auto_mode = TRUE,
    },
};

#endif

