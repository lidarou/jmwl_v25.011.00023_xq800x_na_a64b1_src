/*****************************************************************************/
/*                                                                           */
/*   Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of gpo module.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a6b67
#define LOG_MODULE    AMOD_DRV

#include <aic_bitmap.h>
#include <aic_io.h>
#include <core_op.h>
#include <slp_inc.h>

#include <gpo_cfg.h>
#include <gpo_inc.h>

#define GPO_PIN_CFG_MAX         16

struct gpo_config {
    u32 pin;
    GPO_MODE mod;
};

struct gpo_config g_gpo_cfg[GPO_PIN_CFG_MAX];
u32 g_gpo_cfg_cnt;

static void gic_gpo_phy_mode_cfg(u32 gpo_base, GPO_MODE mode)
{
    switch (mode) {
    case GPO_OUTPUT_LOW:
        AIC_REG_CLRVAL(gpo_base, GPO_PIN_VAL);
        AIC_REG_SETVAL(gpo_base, GPO_PIN_EB);
        break;
    case GPO_OUTPUT_HIGH:
        AIC_REG_SETVAL(gpo_base, GPO_PIN_VAL);
        AIC_REG_SETVAL(gpo_base, GPO_PIN_EB);
        break;
    case GPO_DISABLE:
        AIC_REG_CLRVAL(gpo_base, GPO_PIN_EB);
        break;
    default:
        AIC_ASSERT(0);
    }
}

void aic_gpo_phy_output(u32 gpo_id, GPO_MODE mode)
{
    u32 i, base, id_max = sizeof(g_gpo_map) / sizeof(*g_gpo_map);
    BOOL requested = FALSE;
    AIC_INT_SAVE_AREA

    AIC_ASSERT((gpo_id < id_max) && (mode <= GPO_DISABLE));

    base = g_gpo_map[gpo_id];

    AIC_INT_DISABLE;

    for (i = 0; i < GPO_PIN_CFG_MAX; i++) {
        if (g_gpo_cfg[i].pin == base) {
            requested = TRUE;
            break;
        }
    }

    gic_gpo_phy_mode_cfg(base, mode);

    if (requested == TRUE) {
        switch (mode) {
        case GPO_OUTPUT_LOW:
        case GPO_OUTPUT_HIGH:
            g_gpo_cfg[i].pin = base;
            g_gpo_cfg[i].mod = mode;
            break;
        case GPO_DISABLE:
            g_gpo_cfg[i].pin = 0;
            g_gpo_cfg[i].mod = 0;
            break;
        default:
            AIC_ASSERT(0);
        }
    } else {
        g_gpo_cfg[g_gpo_cfg_cnt].pin = base;
        g_gpo_cfg[g_gpo_cfg_cnt++].mod = mode;
        AIC_ASSERT(g_gpo_cfg_cnt < GPO_PIN_CFG_MAX);
    }
    AIC_INT_RESTORE
}

GPO_MODE aic_gpo_phy_get_level(u32 gpo_id)
{
    u32 val, base, id_max = sizeof(g_gpo_map) / sizeof(*g_gpo_map);

    AIC_ASSERT(gpo_id < id_max);

    base = g_gpo_map[gpo_id];
    val = AIC_READ32(base);

    if (!(val & GPO_PIN_EB))
        return GPO_DISABLE;

    if (val & GPO_PIN_VAL)
        return GPO_OUTPUT_HIGH;
    else
        return GPO_OUTPUT_LOW;
}

void aic_gpo_phy_bare_cfg(u32 pin_base, GPO_MODE mode)
{
    BOOL requested = FALSE;
    u32 i;
    AIC_INT_SAVE_AREA

    AIC_ASSERT(mode <= GPO_DISABLE);

    AIC_INT_DISABLE;

    for (i = 0; i < GPO_PIN_CFG_MAX; i++) {
        if (g_gpo_cfg[i].pin == pin_base) {
            requested = TRUE;
            break;
        }
    }

    gic_gpo_phy_mode_cfg(pin_base, mode);

    if (requested == TRUE) {
        switch (mode) {
        case GPO_OUTPUT_LOW:
        case GPO_OUTPUT_HIGH:
            g_gpo_cfg[i].pin = pin_base;
            g_gpo_cfg[i].mod = mode;
            break;
        case GPO_DISABLE:
            g_gpo_cfg[i].pin = 0;
            g_gpo_cfg[i].mod = 0;
            break;
        default:
            AIC_ASSERT(0);
        }
    } else {
        g_gpo_cfg[g_gpo_cfg_cnt].pin = pin_base;
        g_gpo_cfg[g_gpo_cfg_cnt++].mod = mode;
        AIC_ASSERT(g_gpo_cfg_cnt < GPO_PIN_CFG_MAX);
    }
    AIC_INT_RESTORE
}

static void aic_gpo_phy_resume(void *p)
{
    u32 i;

    for (i = 0; i < GPO_PIN_CFG_MAX; i++) {
        if (!g_gpo_cfg[i].pin)
            continue;
        if (g_gpo_cfg[i].mod)
            AIC_REG_SETVAL(g_gpo_cfg[i].pin, GPO_PIN_VAL);
        else
            AIC_REG_CLRVAL(g_gpo_cfg[i].pin, GPO_PIN_VAL);
        AIC_REG_SETVAL(g_gpo_cfg[i].pin, GPO_PIN_EB);
    }
}

void aic_gpo_phy_init(void)
{
    slp_register_sleep_hook(NULL, NULL, aic_gpo_phy_resume, NULL);
}

