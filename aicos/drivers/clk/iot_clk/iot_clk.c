/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines clock driver functions.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x017607
#define LOG_MODULE    AMOD_DRV

#include <aic_iot_clk.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_clk.h>
#include <timer_inc.h>

#if defined(CONFIG_SUBSYS_AP)
#include <reg_base.h>
#include <aon_glb.h>
#endif

aos_mutex_ptr aic_iot_clk_mutex = AOS_NULL;

static int iot_clk_lock(void)
{
    int ret;

    ret = aos_get_mutex(aic_iot_clk_mutex, AOS_WAIT_FOREVER);

    return ret;
}

static void iot_clk_unlock(void)
{
    aos_put_mutex(aic_iot_clk_mutex);
}

/* clk set interface */
int clk_set_parent_mux(u8 clk_id, u8 index)
{
    struct clk_mux *mux;
    u32 val;

    if (clk_id >= sizeof(clk_mux_array) / sizeof(clk_mux_array[0]))
        return AIC_EINVAL;

    if (clk_mux_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    mux = &clk_mux_array[clk_id];

    val = readl_relaxed(&mux->reg);

    val &= ~GENMASK(mux->mux_shift, mux->mux_width + mux->mux_shift - 1);

    AIC_WRITE32(mux->reg, val | (index << mux->mux_shift));

    mux->cur_parent_index = index;

    return 0;
}

int clk_set_parent_comp(u8 clk_id, u8 index)
{
    struct clk_comp *comp;
    u32 val;

    if (clk_id >= sizeof(clk_comp_array) / sizeof(clk_comp_array[0]))
        return AIC_EINVAL;

    if (clk_comp_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    comp = &clk_comp_array[clk_id];

    val = AIC_READ32(comp->reg);
    val &= ~GENMASK(comp->mux_shift, comp->mux_width + comp->mux_shift - 1);

    AIC_WRITE32(comp->reg, val | (index << comp->mux_shift));

    comp->cur_parent_index = index;

    return 0;
}

int clk_set_rate_comp(u8 clk_id, u32 rate)
{
    struct clk_comp *comp;
    u32 val, parent_rate;

    if (clk_id >= sizeof(clk_comp_array) / sizeof(clk_comp_array[0]))
        return AIC_EINVAL;

    if (clk_comp_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    comp = &clk_comp_array[clk_id];

    parent_rate = comp->parents[comp->cur_parent_index].rate;
    if (rate > parent_rate)
        return AIC_EOVERFLOW;

    val = AIC_READ32(comp->reg);
    val &= ~GENMASK(comp->div_shift, comp->div_width + comp->div_shift - 1);

    AIC_WRITE32(comp->reg, val | ((parent_rate/rate - 1) << comp->div_shift));

    return 0;
}

int clk_set_rate_div(u8 clk_id, u32 rate)
{
    struct clk_div *div;
    u32 val, parent_rate;

    if (clk_id >= sizeof(clk_div_array) / sizeof(clk_div_array[0]))
        return AIC_EINVAL;

    if (clk_div_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    div = &clk_div_array[clk_id];

    parent_rate = div->prate;

    if (rate > parent_rate)
        return AIC_EOVERFLOW;

    val = AIC_READ32(div->reg);
    val &= ~GENMASK(div->div_shift, div->div_width + div->div_shift - 1);

    AIC_WRITE32(div->reg, val | ((parent_rate/rate - 1) << div->div_shift));

    return 0;
}

/* gate clock interface */
int clk_enable(u8 clk_id)
{
    struct clk_gate *gate;

    if (clk_id >= sizeof(clk_gate_array) / sizeof(clk_gate_array[0]))
        return AIC_EINVAL;

    if (clk_gate_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    gate = &clk_gate_array[clk_id];
    AIC_WRITE32((gate->reg + 0x1000), BIT(gate->index));

    return 0;
}

int clk_disable(u8 clk_id)
{
    struct clk_gate *gate;

    if (clk_id >= sizeof(clk_gate_array) / sizeof(clk_gate_array[0]))
        return AIC_EINVAL;

    if (clk_gate_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    gate = &clk_gate_array[clk_id];
    AIC_WRITE32((gate->reg + 0x2000), BIT(gate->index));

    return 0;
}

/* pll gate clock interface */
int clk_pll_enable(u8 clk_id)
{
    int ret;
    struct clk_pll_gate *pll_gate;

    if (clk_id >= sizeof(clk_pll_gate_array) / sizeof(clk_pll_gate_array[0]))
        return AIC_EINVAL;

    if (clk_pll_gate_array[0].gate.clk_id == 0xFF)
        return AIC_EINVAL;

    ret = iot_clk_lock();
    if (ret != AIC_SUCCESS) {
        return AIC_ENOLCK;
    }

    pll_gate = &clk_pll_gate_array[clk_id];
    if (pll_gate->enable_cnt == 0) {
        AIC_WRITE32(pll_gate->gate.reg, AIC_READ32(pll_gate->gate.reg) | BIT(pll_gate->gate.index));
        /* wait 100us for pll power up */
        if (pll_gate->delay_time)
            aic_udelay(pll_gate->delay_time);
    }

    pll_gate->enable_cnt++;

    iot_clk_unlock();

    return 0;

}

int clk_pll_disable(u8 clk_id)
{
    int ret;
    struct clk_pll_gate *pll_gate;

    if (clk_id >= sizeof(clk_pll_gate_array) / sizeof(clk_pll_gate_array[0]))
        return AIC_EINVAL;

    if (clk_pll_gate_array[0].gate.clk_id == 0xFF)
        return AIC_EINVAL;

    ret = iot_clk_lock();
    if (ret != AIC_SUCCESS) {
        return AIC_ENOLCK;
    }

    pll_gate = &clk_pll_gate_array[clk_id];
    if (pll_gate->enable_cnt != 0)
        pll_gate->enable_cnt--;

    if (pll_gate->enable_cnt == 0)
        AIC_WRITE32(pll_gate->gate.reg, AIC_READ32(pll_gate->gate.reg) & ~BIT(pll_gate->gate.index));

    iot_clk_unlock();

    return 0;

}

/* parent get interface */
u8 clk_get_parent_mux(u8 clk_id)
{
    struct clk_mux *mux;

    if (clk_id >= sizeof(clk_mux_array) / sizeof(clk_mux_array[0]))
        return AIC_EINVAL;

    if (clk_mux_array[0].clk_id == 0xFF)
        return AIC_EINVAL;


    mux = &clk_mux_array[clk_id];

    return mux->cur_parent_index;
}

u8 clk_get_parent_comp(u8 clk_id)
{
    struct clk_comp *comp;

    if (clk_id >= sizeof(clk_comp_array) / sizeof(clk_comp_array[0]))
        return AIC_EINVAL;

    if (clk_comp_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    comp = &clk_comp_array[clk_id];

    return comp->cur_parent_index;
}

/* rate get interface */
u32 clk_get_rate_comp(u8 clk_id)
{
    struct clk_comp *comp;
    u32 val;

    if (clk_id >= sizeof(clk_comp_array) / sizeof(clk_comp_array[0]))
        return AIC_EINVAL;

    if (clk_comp_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    comp = &clk_comp_array[clk_id];

    val = AIC_READ32(comp->reg);
    val &= GENMASK(comp->div_width, comp->div_width + comp->div_shift - 1);
    val = val >> comp->div_shift;

    return comp->parents[comp->cur_parent_index].rate / val;
}

u32 clk_get_rate_mux(u8 clk_id)
{
    struct clk_mux *mux;

    if (clk_id >= sizeof(clk_mux_array) / sizeof(clk_mux_array[0]))
        return AIC_EINVAL;

    if (clk_mux_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    mux = &clk_mux_array[clk_id];

    return mux->parents[mux->cur_parent_index].rate;
}

u32 clk_get_rate_div(u8 clk_id)
{
    struct clk_div *div;
    u32 val;

    if (clk_id >= sizeof(clk_div_array) / sizeof(clk_div_array[0]))
        return AIC_EINVAL;

    if (clk_div_array[0].clk_id == 0xFF)
        return AIC_EINVAL;

    div = &clk_div_array[clk_id];

    val = AIC_READ32(div->reg);
    val &= GENMASK(div->div_width, div->div_width + div->div_shift - 1);
    val = val >> div->div_shift;

    return div->prate / val;
}

int iot_clk_init(void)
{
    /* create a mutex for gate clock */
    aic_iot_clk_mutex = aos_create_mutex("clk");
    AIC_ASSERT(aic_iot_clk_mutex);

#if defined(CONFIG_SUBSYS_AP) && defined(REG_AON_GLB_UPLL_24M_CKEN_FORCE_CLR)
    /* disable camera and usb's 24m clk */
    AIC_WRITE32(REG_AON_GLB_UPLL_24M_CKEN_FORCE_CLR, BIT_AON_GLB_UPLL_CKEN_CKM_BUSY_24M_FORCE);
#endif

    return 0;
}
AIC_EARLY_INIT_EXPORT(iot_clk_init);

