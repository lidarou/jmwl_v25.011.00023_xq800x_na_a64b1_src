/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of gpd module, it is gpd
 *    common hal driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1704d4
#define LOG_MODULE    AMOD_GPD

#include <aic_auto_reg.h>
#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_clk_map.h>
#include <aic_cmd_api.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_iot_clk.h>
#include <aic_type.h>
#include <irq.h>
#include <irq_chip.h>
#include <gpd_inc.h>
#include <stdlib.h>

#include "gpd_interface.h"

/* gpd default counter 40000 */
#define AIC_GPD_DEFALUT_EDGE_CNT     0x9c40

struct gpd_cnt_info {
    u32 edge_cnt;
    u32 cycle_cnt;
    u32 total_edge;
    u32 total_cycle;
    u64 threshold_cnt;
};
static struct gpd_cnt_info g_gpd_info[GPD_COUNTER_MAX];

void aic_gpd_update(GPD_INDEX index)
{
    aic_gpd_phy_update(index);
}

void aic_gpd_mode_cfg(GPD_INDEX index, GPD_EDGE_TYPE mode)
{
    AIC_ASSERT((index < GPD_COUNTER_MAX) && (mode <= GPD_BOTHEDGE_TYPE));
    aic_gpd_phy_edge_type_cfg(index, mode);
}

void aic_gpd_cnt_cfg(GPD_INDEX index, u32 cnt)
{
    aic_gpd_phy_edge_cnt_set(index, cnt);
    g_gpd_info[index].edge_cnt = cnt;
}

u32 aic_gpd_get_current_cnt(GPD_INDEX index)
{
    return aic_gpd_phy_edge_cnt_get(index);
}

u32 aic_gpd_get_cnt(GPD_INDEX index)
{
    return g_gpd_info[index].edge_cnt;
}

u32 aic_gpd_get_cycle(GPD_INDEX index)
{
    return g_gpd_info[index].cycle_cnt;
}

ISR_RET_E aic_gpd_irq_handler(void *param)
{
    u32 val, index, cycle_val;

    val = aic_gpd_phy_get_int_state();
    AIC_ASSERT(val);

    for (index = 0; val != 0; val &= ~(1 << index)) {
        index = aic_find_first_bit(val);
        AIC_ASSERT(index < GPD_COUNTER_MAX);
        cycle_val = aic_gpd_phy_cycle_cnt_get(index);
        g_gpd_info[index].cycle_cnt = cycle_val;
        g_gpd_info[index].total_edge += g_gpd_info[index].edge_cnt;
        g_gpd_info[index].total_cycle += cycle_val;
        aic_gpd_phy_cycle_clr(index);
        aic_gpd_phy_int_clr(index);
        if (g_gpd_info[index].total_edge >= g_gpd_info[index].threshold_cnt)
            aicx_info(0x083f04, "dd", "-- GPD %d - %d\n", index, g_gpd_info[index].total_edge);
    }

    return NONE_WORKER;
}

/*
 * gpd_config - use to config gpd in console
 *
 * glitch: config glitch filter counter
 * type:   config edge type
 * cnt:    config counter max to trigger interrupt
 */
int aic_gpd_config(int argc, char **argv)
{
    u32 val, i;

    if (!strcmp(argv[0], "glitch")) {
        val = atoi(argv[1]);
        for (i = GPD_COUNTER0; i < GPD_COUNTER_MAX; i++)
            aic_gpd_phy_glitch_set(i, val);
    }
    if (!strcmp(argv[0], "cnt")) {
        val = atoi(argv[1]);
        for (i = GPD_COUNTER0; i < GPD_COUNTER_MAX; i++) {
            g_gpd_info[i].threshold_cnt = val;
            if (val >= GPD_CNT_CFG_MAX)
                aic_gpd_cnt_cfg(i, GPD_CNT_CFG_MAX);
            else
                aic_gpd_cnt_cfg(i, val);
        }
    }
    if (!strcmp(argv[0], "type")) {
        val = atoi(argv[1]);
        for (i = GPD_COUNTER0; i < GPD_COUNTER_MAX; i++)
            aic_gpd_mode_cfg(i, val);
    }
    if (!strcmp(argv[0], "read")) {
        for (i = GPD_COUNTER0; i < GPD_COUNTER_MAX; i++) {
            val = aic_gpd_get_current_cnt(i);
            aicx_info(0x12a914, "dd", "-- GPD %d cur cnt %d\n", i, val);
        }
    }
    if (!strcmp(argv[0], "enable")) {
        val = atoi(argv[1]);
        switch (val) {
        case 0:
            for (i = GPD_COUNTER0; i < GPD_COUNTER_MAX; i++) {
                aic_gpd_phy_update(i);
                aic_gpd_phy_disable(i);
            }
            break;
        case 1:
            for (i = GPD_COUNTER0; i < GPD_COUNTER_MAX; i++) {
                aic_gpd_phy_update(i);
                aic_gpd_phy_enable(i);
            }
            break;
        default:
            //Do Nothing
            break;
        }
    }
    if (!strcmp(argv[0], "clk")) {
        val = atoi(argv[1]);
        switch (val) {
        case 0:
            /* 0:100M 1:26M 2:32K */
            clk_set_parent_comp(CLK_AON_GPD, 0);
            break;
        case 1:
            /* 0:100M 1:26M 2:32K */
            clk_set_parent_comp(CLK_AON_GPD, 1);
            break;
        case 2:
            /* 0:100M 1:26M 2:32K */
            clk_set_parent_comp(CLK_AON_GPD, 2);
            break;
        default:
            //Do Nothing
            break;
        }
    }

    return 0;
}
CONSOLE_EXPORT_CMD(gpd, aic_gpd_config, "gpd config");

int aic_gpd_init(void)
{
    register_irq(IRQ_GPD, aic_gpd_irq_handler, NULL, NULL, NULL);

    /* set gpd glb */
    AIC_WRITE32(REG_AON_GLB_GPD_EB, BIT_AON_GLB_GPD_EB);

    /* init gpd */
    aic_gpd_phy_common_init();

    /* 0:100M 1:26M 2:32K */
    clk_set_parent_comp(CLK_AON_GPD, 0);

    return 0;
}

AIC_DRV_P2_INIT_EXPORT(aic_gpd_init);

