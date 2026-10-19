/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18b5ba
#define LOG_MODULE    AMOD_PMIC

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_type.h>
#include <aic_iot_clk.h>
#include <pmic_io.h>
#include <aic_pmic_clk.h>

/* gate clock interface */
int pmic_clk_enable(u8 clk_id)
{
    struct clk_gate *gate;

    gate = &pmic_clk_gate_array[clk_id];
    pmic_writel(gate->reg, BIT(gate->index));

    return 0;
}

int pmic_clk_disable(u8 clk_id)
{
    u32 val;
    struct clk_gate *gate;

    gate = &pmic_clk_gate_array[clk_id];
    val = pmic_readl(gate->reg);
    val &= ~BIT(gate->index);
    pmic_writel(gate->reg, val);

    return 0;
}

