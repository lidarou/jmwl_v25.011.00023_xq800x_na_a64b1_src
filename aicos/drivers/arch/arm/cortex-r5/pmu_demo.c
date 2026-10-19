/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file demo how to use performance monitor counter.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1964c8
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <core_op.h>
#include <alog_api.h>
#include <arch/arm/cortex-r5/pmu.h>

/*
 * PMU: there are 3 counters and count 3 events once.
 * Please reference PmcrEvents in pmu.c
 */

void pmu_demo(void)
{
    u64 l1_cnts[CTRCOUNT];

    /* configure l1 pmu cnts */
    pmu_setup_cnts(1);
/************************************
 * Insert youre code to be measured *
 ************************************/
    /* get pmu vals */
    pmu_get_cntsvals(l1_cnts);
    aicx_info(0x071684, "lxlxlx", "L1 cnt0 0x%lx, cnt1 0x%lx, cnt2 0x%lx\n",
             l1_cnts[0], l1_cnts[1], l1_cnts[2]);
}