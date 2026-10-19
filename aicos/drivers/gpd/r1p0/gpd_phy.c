/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of gpd module.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d3287
#define LOG_MODULE    AMOD_GPD

#include <aic_auto_reg.h>
#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <core_op.h>

#include <gpd_inc.h>
#include <gpd_reg.h>

void aic_gpd_phy_enable(GPD_INDEX index)
{
    AIC_REG_SETVAL(REG_GPD_GLB_CTRL, BIT_GPD0_EN << index);
}

void aic_gpd_phy_disable(GPD_INDEX index)
{
    AIC_REG_CLRVAL(REG_GPD_GLB_CTRL, BIT_GPD0_EN << index);
}

void aic_gpd_phy_update(GPD_INDEX index)
{
    AIC_REG_SETVAL(REG_GPD_UPDATE, BIT_GPD0_UPDATE << index);
}

void aic_gpd_phy_cnt_clr(GPD_INDEX index)
{
    AIC_REG_SETVAL(REG_GPD_UPDATE, BIT_GPD0_EDGE_CNT_CLR << index);
}

void aic_gpd_phy_cycle_clr(GPD_INDEX index)
{
    AIC_REG_SETVAL(REG_GPD_UPDATE, BIT_GPD0_EDGE_CYCLE_CLR << index);
}

void aic_gpd_phy_common_init(void)
{
    AIC_WRITE32(REG_GPD_INT_EN, BIT_GPD0_INT_EN | BIT_GPD1_INT_EN |
                                BIT_GPD2_INT_EN | BIT_GPD3_INT_EN);
}

void aic_gpd_phy_edge_type_cfg(GPD_INDEX index, GPD_EDGE_TYPE type)
{
    u32 val;

    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

    val = AIC_READ32(REG_GPD_EDGE_TYPE);
    switch (index) {
    case GPD_COUNTER0:
        val &= ~BIT_GPD0_EDGE_TYPE(0x3);
        val |= BIT_GPD0_EDGE_TYPE(type);
        break;
    case GPD_COUNTER1:
        val &= ~BIT_GPD1_EDGE_TYPE(0x3);
        val |= BIT_GPD1_EDGE_TYPE(type);
        break;
    case GPD_COUNTER2:
        val &= ~BIT_GPD2_EDGE_TYPE(0x3);
        val |= BIT_GPD2_EDGE_TYPE(type);
        break;
    case GPD_COUNTER3:
        val &= ~BIT_GPD3_EDGE_TYPE(0x3);
        val |= BIT_GPD3_EDGE_TYPE(type);
        break;
    default:
        AIC_ASSERT(0);
    }
    AIC_WRITE32(REG_GPD_EDGE_TYPE, val);

    AIC_INT_RESTORE
}

void aic_gpd_phy_glitch_set(GPD_INDEX index, u32 cnt)
{
    u32 val;

    AIC_ASSERT(cnt <= BIT_GPD_GLITCH_CNT_MASK);

    val = AIC_READ32(REG_GPD_CNT(index));
    val = (val & BIT_GPD_EDGA_CNT_MASK) +
            BIT_GPD0_101_GLITCH_FILTER_CNT(cnt) +
            BIT_GPD0_010_GLITCH_FILTER_CNT(cnt);
    AIC_WRITE32(REG_GPD_CNT(index), val);
}

void aic_gpd_phy_edge_cnt_set(GPD_INDEX index, u32 cnt)
{
    u32 val;

    AIC_ASSERT(cnt <= BIT_GPD0_EDGE_CNT_MAX(0xFFFF));

    val = AIC_READ32(REG_GPD_CNT(index));
    /* -1? fix ip bug, the interrupt will trigger more than one max cnt */
    val = (val & ~BIT_GPD_EDGA_CNT_MASK) + cnt - 1;
    AIC_WRITE32(REG_GPD_CNT(index), val);
}

u32 aic_gpd_phy_edge_cnt_get(GPD_INDEX index)
{
    return AIC_READ32(REG_GPD_EDGE(index));
}

u32 aic_gpd_phy_cycle_cnt_get(GPD_INDEX index)
{
    return AIC_READ32(REG_GPD_CYCLE_CNT(index));
}

u32 aic_gpd_phy_get_int_state(void)
{
    return (AIC_READ32(REG_GPD_INT_STATUS) & BIT_GPD_INT_MASK);
}

void aic_gpd_phy_int_clr(GPD_INDEX index)
{
    AIC_REG_SETVAL(REG_GPD_INT_STATUS_CLR, BIT(index));
    AIC_REG_CLRVAL(REG_GPD_INT_STATUS_CLR, BIT(index));
}

