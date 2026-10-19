/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the pmic efuse software driver.
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08cd2c
#define LOG_MODULE    AMOD_PMIC

#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "pmic_efuse_reg.h"
#include "../pmic_efuse_phy.h"
#include <timer_inc.h>

void (*pmic_efuse_enable)(bool enabled);
#define GP202_PD_CKM_EN_GLB_CLK_PD_EFUSE_RF (0x801C)

static int aic_pmic_efuse_wait_int_finish(AIC_PMIC_EFUSE_INT efuse_int)
{
    int i, ret;
    u32 val;

    ret = 0;
    i = 0;

    while( i < PMIC_EFUSE_WAIT_TIME_CNT) {
        i ++;
        val = pmic_readl(REG_PMIC_EFUSE_INT_STATUS) & efuse_int;

        if (val)
            break;
        aos_sleep(1);
    }

    if(i == PMIC_EFUSE_WAIT_TIME_CNT)
        ret = AIC_ENOTSUP;

    return ret;
}

static void aic_pmic_efuse_int_status_clr(AIC_PMIC_EFUSE_INT efuse_int)
{
    pmic_writel(REG_PMIC_EFUSE_INT_STATUS_CLR, efuse_int);
}

static void aic_pmic_efuse_int_enable(AIC_PMIC_EFUSE_INT efuse_int)
{
    u32 val;

    val = pmic_readl(REG_PMIC_EFUSE_INT_ENABLE);
    val = val | efuse_int;
    pmic_writel(REG_PMIC_EFUSE_INT_ENABLE, val);
}

static void aic_pmic_efuse_int_disable(AIC_PMIC_EFUSE_INT efuse_int)
{
    u32 val;

    val = pmic_readl(REG_PMIC_EFUSE_INT_ENABLE);
    val = val & ~efuse_int;//To be check
    pmic_writel(REG_PMIC_EFUSE_INT_ENABLE, val);
}

static void aic_pmic_efuse_write_unlock(void)
{
    pmic_writel(REG_PMIC_EFUSE_MAGIC_DATA, BIT_PMIC_EFUSE_MAGIC_DATA_WR(PMIC_EFUSE_WRITE_MAGIC));
}

static void aic_pmic_efuse_write_lock(void)
{
    pmic_writel(REG_PMIC_EFUSE_MAGIC_DATA, BIT_PMIC_EFUSE_MAGIC_DATA_WR(0));
}

static void aic_pmic_efuse_blk_rw_trigger(void)
{
    pmic_writel(REG_PMIC_EFUSE_RW_TRIGGER, BIT_PMIC_EFUSE_RW_TRIGGER);
}

static void aic_pmic_efuse_write_blk_pre(int blk_id, u32 date)
{
    u32 val;

    /*enable write mode & set blk id*/
    val = BIT_PMIC_EFUSE_ADDR(blk_id) | BIT_PMIC_EFUSE_WRITE_EN;
    pmic_writel(REG_PMIC_EFUSE_RW_CTRL, val);

    pmic_writel(REG_PMIC_EFUSE_WRITE_DATA, date);
}

static void aic_pmic_efuse_read_blk_pre(int blk_id)
{
    u32 val;

    /*disable write mode & set blk id*/
    val = BIT_PMIC_EFUSE_ADDR(blk_id) & ~BIT_PMIC_EFUSE_WRITE_EN;
    pmic_writel(REG_PMIC_EFUSE_RW_CTRL, val);
}

static u32 aic_pmic_efuse_read_blk_post()
{
    return pmic_readl(REG_PMIC_EFUSE_READ_DATA);
}

static void aic_pmic_efuse_enable(void)
{
    u32 val;

    pmic_efuse_enable(true);

    val = pmic_readl(REG_PMIC_EFUSE_EN_CTRL);
    pmic_writel(REG_PMIC_EFUSE_EN_CTRL, val | BIT_PMIC_EFUSE_EN);
}

static void aic_pmic_efuse_disable(void)
{
    u32 val;

    val = pmic_readl(REG_PMIC_EFUSE_EN_CTRL);
    pmic_writel(REG_PMIC_EFUSE_EN_CTRL, val & ~ BIT_PMIC_EFUSE_EN);

    pmic_efuse_enable(false);
}

#define GP201_REG_GLB_PD_CKM_EN_GLB_CLK_APB_EFUSE_RF    0x8014
#define GP201_REG_GLB_PD_CKM_EN_GLB_CLK_PD_EFUSE_RF     0x8024
void gp201_efuse_enable(bool enabled)
{
    if (enabled) {
        pmic_writel(GP201_REG_GLB_PD_CKM_EN_GLB_CLK_APB_EFUSE_RF, BIT(0));
        pmic_writel(GP201_REG_GLB_PD_CKM_EN_GLB_CLK_PD_EFUSE_RF, BIT(0));
    } else {
        pmic_writel(GP201_REG_GLB_PD_CKM_EN_GLB_CLK_APB_EFUSE_RF, 0x0);
        pmic_writel(GP201_REG_GLB_PD_CKM_EN_GLB_CLK_PD_EFUSE_RF, 0x0);
    }
}

void gp202_efuse_enable(bool enabled)
{
    if (enabled) {
        pmic_writel(GP202_PD_CKM_EN_GLB_CLK_PD_EFUSE_RF, BIT(0));
    } else {
        pmic_writel(GP202_PD_CKM_EN_GLB_CLK_PD_EFUSE_RF, 0x0);
    }
}

int aic_pmic_efuse_write(int blk, u32 data)
{
    int ret = 0;

    aic_pmic_efuse_enable();
    aic_pmic_efuse_int_enable(AIC_PMIC_EFUSE_ALL);
    aic_pmic_efuse_write_unlock();
    aic_pmic_efuse_write_blk_pre(blk, data);
    aic_pmic_efuse_blk_rw_trigger();

    ret = aic_pmic_efuse_wait_int_finish(AIC_PMIC_EFUSE_WRITE_FINISH);
    if (ret) {
        aicx_error(0x0b6f0f, "", "efuse write finish timeout!\n");
        goto out;
    }

    aic_pmic_efuse_write_lock();
    aic_pmic_efuse_int_status_clr(AIC_PMIC_EFUSE_ALL);
    aic_pmic_efuse_int_disable(AIC_PMIC_EFUSE_ALL);
    aic_pmic_efuse_disable();
out:
    return ret;
}

int aic_pmic_efuse_read(int blk,u32 *data)
{
    int ret;

    aic_pmic_efuse_enable();
    aic_pmic_efuse_int_enable(AIC_PMIC_EFUSE_ALL);
    aic_pmic_efuse_read_blk_pre(blk);
    aic_pmic_efuse_blk_rw_trigger();

    ret = aic_pmic_efuse_wait_int_finish(AIC_PMIC_EFUSE_READ_FINISH);
    if (ret) {
        aicx_error(0x00d26d, "", "efuse read finish timeout!\n");
        goto out;
    }

    *data = aic_pmic_efuse_read_blk_post();

    aic_pmic_efuse_int_status_clr(AIC_PMIC_EFUSE_ALL);
    aic_pmic_efuse_int_disable(AIC_PMIC_EFUSE_ALL);
    aic_pmic_efuse_disable();

out:
    return ret;
}

int efuse_phy_init(void)
{
    int ret =0, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    pmic_id = ((pmic_id >> 16) & 0xFFFF);

    if (pmic_id == 0x201)
        pmic_efuse_enable = gp201_efuse_enable;
    else
        pmic_efuse_enable = gp202_efuse_enable;

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(efuse_phy_init);
