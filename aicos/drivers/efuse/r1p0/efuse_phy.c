/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the efuse software driver, it is efuse
 *    r1p0 phy driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1ca7c2
#define LOG_MODULE    AMOD_DRV

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_osa.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include "../efuse_phy.h"
#include "efuse_reg.h"
#include <timer_inc.h>

void aic_efuse_blk_property_scan(struct efuse_blk_info *efuse_fields)
{
    u32 sip_blk0, sip_blk1, sec_blk0, sec_blk1;
    int i;

    sip_blk0 = AIC_READ32(REG_EFUSE_SIP_BLK_INFO0);
    sip_blk1 = AIC_READ32(REG_EFUSE_SIP_BLK_INFO1);
    sec_blk0 = AIC_READ32(REG_EFUSE_SECURITY_BLK_INFO0);
    sec_blk1 = AIC_READ32(REG_EFUSE_SECURITY_BLK_INFO1);

    for (i = 0; i < EFUSE_BLK_NUMS; i ++) {
        if (i < 32) {
            if (sip_blk0 & BIT(i))
                efuse_fields[i].blk_property = AIC_SEC_IP_BLK;
            else if (sec_blk0 & BIT(i))
                efuse_fields[i].blk_property = AIC_SECURITY_BLK;
            else
                efuse_fields[i].blk_property = AIC_GENERAL_BLK;
        } else {
            if (sip_blk1 & BIT(i - 32))
                efuse_fields[i].blk_property = AIC_SEC_IP_BLK;
            else if (sec_blk1 & BIT(i - 32))
                efuse_fields[i].blk_property = AIC_SECURITY_BLK;
            else
                efuse_fields[i].blk_property = AIC_GENERAL_BLK;
        }
    }

    return;
}

static void aic_efuse_int_status_clr(AIC_EFUSE_INT efuse_int)
{
    AIC_WRITE32(REG_EFUSE_EFUSE_INT_STATUS_CLR, efuse_int);
}

static int aic_efuse_wait_int_finish(AIC_EFUSE_INT efuse_int)
{
    int i, ret;
    u32 val;

    ret = 0;
    i = 0;

    while( i < EFUSE_WAIT_TIME_CNT) {
        i ++;
        val = AIC_READ32(REG_EFUSE_EFUSE_INT_STATUS) & efuse_int;

        if (val)
            break;
        aic_mdelay(1);
    }

    aic_efuse_int_status_clr(efuse_int);

    if(i == EFUSE_WAIT_TIME_CNT)
        ret = AIC_ENOTSUP;

    return ret;
}

static void aic_efuse_int_enable(AIC_EFUSE_INT efuse_int)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_INT_ENABLE);
    val = val | efuse_int;
    AIC_WRITE32(REG_EFUSE_EFUSE_INT_ENABLE, val);
}

static void aic_efuse_write_unlock(void)
{
    AIC_WRITE32(REG_EFUSE_EFUSE_MAGIC_DATA, BIT_EFUSE_EFUSE_MAGIC_DATA_WR(EFUSE_WRITE_MAGIC));
}

static void aic_efuse_write_lock(void)
{
    AIC_WRITE32(REG_EFUSE_EFUSE_MAGIC_DATA, BIT_EFUSE_EFUSE_MAGIC_DATA_WR(0));
}

static void aic_efuse_blk_rw_trigger(void)
{
    AIC_WRITE32(REG_EFUSE_EFUSE_RW_TRIGGER, BIT_EFUSE_EFUSE_RW_TRIGGER);
}

static void aic_efuse_write_blk_pre(int blk_id, u32 date)
{
    u32 val;

    /*enable write mode & set blk id*/
    val = BIT_EFUSE_EFUSE_ADDR(blk_id) | BIT_EFUSE_EFUSE_WRITE_EN;
    AIC_WRITE32(REG_EFUSE_EFUSE_RW_CTRL, val);
    AIC_WRITE32(REG_EFUSE_EFUSE_WRITE_DATA, date);
}

static void aic_efuse_read_blk_pre(int blk_id)
{
    u32 val;

    /*disable write mode & set blk id*/
    val = BIT_EFUSE_EFUSE_ADDR(blk_id) & ~BIT_EFUSE_EFUSE_WRITE_EN;
    AIC_WRITE32(REG_EFUSE_EFUSE_RW_CTRL, val);
}

static u32 aic_efuse_read_blk_post()
{
    return AIC_READ32(REG_EFUSE_EFUSE_READ_DATA);
}

static void aic_efuse_powerup(void)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_POWER_CTRL);
    AIC_WRITE32(REG_EFUSE_EFUSE_POWER_CTRL, val & ~BIT_EFUSE_PD_VALUE);
}

static void aic_efuse_powerdown(void)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_POWER_CTRL);
    AIC_WRITE32(REG_EFUSE_EFUSE_POWER_CTRL, val | BIT_EFUSE_PD_VALUE);
}

static void aic_efuse_auto_pd_enabled(bool auto_pd_enabled)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_POWER_CTRL);

    if (auto_pd_enabled)
        AIC_WRITE32(REG_EFUSE_EFUSE_POWER_CTRL, val | BIT_EFUSE_PD_AUTO_EN);
    else
        AIC_WRITE32(REG_EFUSE_EFUSE_POWER_CTRL, val & ~BIT_EFUSE_PD_AUTO_EN);
}

static void aic_efuse_cache_enabled(bool cache_enabled)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_GLB_CTRL);

    if (cache_enabled)
        AIC_WRITE32(REG_EFUSE_EFUSE_GLB_CTRL, val | BIT_EFUSE_EFUSE_CACHE_ENABLE);
    else
        AIC_WRITE32(REG_EFUSE_EFUSE_GLB_CTRL, val & ~BIT_EFUSE_EFUSE_CACHE_ENABLE);
}

static void aic_efuse_enabled(bool efuse_enabled)
{
    u32 val;

    val = AIC_READ32(REG_EFUSE_EFUSE_GLB_CTRL);

    if (efuse_enabled)
        AIC_WRITE32(REG_EFUSE_EFUSE_GLB_CTRL, val | BIT_EFUSE_EFUSE_EN);
    else
        AIC_WRITE32(REG_EFUSE_EFUSE_GLB_CTRL, val & ~BIT_EFUSE_EFUSE_EN);
}

static void aic_efuse_aon_enable(void)
{
    AIC_WRITE32(REG_TOP_GLB_EFUSE_EB, BIT_TOP_GLB_EFUSE_EB);
}

static void aic_efuse_aon_disable(void)
{
    AIC_WRITE32(REG_TOP_GLB_EFUSE_EB, ~BIT_TOP_GLB_EFUSE_EB);
}

int aic_efuse_write(int blk, u32 data)
{
    int ret = 0;

    aic_efuse_phy_probe();
    aic_efuse_powerup();
    aic_efuse_int_enable(AIC_EFUSE_ALL);
    aic_efuse_write_unlock();
    aic_efuse_write_blk_pre(blk, data);
    aic_efuse_blk_rw_trigger();

    ret = aic_efuse_wait_int_finish(AIC_EFUSE_WRITE_FINISH);
    if (ret) {
        goto out;
    }
    aic_efuse_write_lock();
    aic_efuse_int_status_clr(AIC_EFUSE_ALL);

out:
    aic_efuse_powerdown();
    return ret;
}

int aic_efuse_read(int blk,u32 *data)
{
    int ret;

    aic_efuse_phy_probe();
    aic_efuse_powerup();
    aic_efuse_int_enable(AIC_EFUSE_ALL);
    aic_efuse_read_blk_pre(blk);
    aic_efuse_blk_rw_trigger();

    ret = aic_efuse_wait_int_finish(AIC_EFUSE_READ_FINISH);
    if (ret) {
        goto out;
    }

    *data = aic_efuse_read_blk_post();
    aic_efuse_int_status_clr(AIC_EFUSE_ALL);

out:
    aic_efuse_powerdown();
    return ret;
}

void aic_efuse_phy_probe(void)
{
    aic_efuse_aon_enable();
    aic_efuse_enabled(TRUE);
    aic_efuse_auto_pd_enabled(AIC_AUTO_PD_ENABLED);
    aic_efuse_cache_enabled(AIC_CACHE_ENABLED);
}

void aic_efuse_phy_remove(void)
{
    aic_efuse_enabled(FALSE);
    aic_efuse_aon_disable();
}
