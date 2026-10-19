/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  pmu subsys kit interface.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15c841
#define LOG_MODULE    AMOD_PMIC

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pmu_driver.h>
#include <pmu_kit.h>
#include <timer_inc.h>

int pmu_io_update(const struct pmu_io *pio, u32 pmu_reg, u32 pmu_msk,
                            u32 pmu_val)
{
    u32 val;
    int ret;

    if (!pmu_msk) {
            aicx_warn(0x0f4a00, "", "illegal mask value input!\n");
        return AIC_EINVAL;
    }

    ret = pio->pmu_read(pmu_reg, &val);
    if(ret) {
        aicx_warn(0x18e916, "", "pmu io read fail!\n");
        return AIC_EIO;
    }

    val &= ~pmu_msk;

    pmu_val <<= aic_find_first_bit(pmu_msk);

    val |= pmu_val;

    pmu_dbg("%s %d %s: pmu_reg:0x%x val:0x%x\n",
                __FILE__, __LINE__, __func__, pmu_reg, val);

    ret = pio->pmu_write(pmu_reg, val);
    if(ret)
        aicx_warn(0x18e916, "", "pmu io read fail!\n");

    return ret;
}

int pmu_kit_set_current_table(struct pmu_unit *punit, u32 current_uA,
                                    u32 tolerance_uA)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    u32 current_steps = pinfo->current_step_num;
    int current_min = current_uA - tolerance_uA;
    int current_max = current_uA + tolerance_uA;
    int i, ret = 0, sel = -1;

    if (current_steps == 0) {
        ret = AIC_EIO;
        goto out;
    }

    if (current_min < 0)
        current_min = 0;

    pmu_dbg("%s %d %s: %s current_min:%d current_max:%d\n",
                __FILE__, __LINE__, __func__, pinfo->unit_name,
                current_min, current_max);

    if (pinfo->current_table) {
        const uint32_t *curr_table = pinfo->current_table;
        bool trend = curr_table[current_steps - 1] > curr_table[0];

        /*closest to maximum*/
        if (trend) {
            for (i = current_steps - 1; i >= 0; i--) {
                if (current_min <= curr_table[i] && curr_table[i] < current_max) {
                    sel = i;
                    break;
                }
            }
        } else {
            for ( i = 0; i < current_steps; i ++) {
                if (current_min <= curr_table[i] && curr_table[i] < current_max) {
                    sel = i;
                    break;
                }
            }
        }
    }

    if (sel < 0) {
        aicx_warn(0x10b9a6, "", "failed:didn't match any sel in current table!\n");
        ret = AIC_EIO;
        goto out;
    }

    ret = pmu_io_update(pio, pinfo->current_reg, pinfo->current_msk, sel);
    if (ret) {
        aicx_warn(0x15fec8, "s", "%s pmu write current fail!\n", __func__);
        goto out;
    }

    ret = pmu_io_update(pio, pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        aicx_warn(0x1a9029, "s", "%s pmu update config fail!\n", __func__);

    aic_udelay(pinfo->scaling_time_us);

out:
    return ret;
}

int pmu_kit_get_current_table(struct pmu_unit *punit, u32 *current_uA)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    u32 current_steps = pinfo->current_step_num;
    int ret;
    u32 val;

    pmu_dbg("%s %d %s: %s\n", __FILE__, __LINE__, __func__, pinfo->unit_name);

    if (pio == NULL) {
        aicx_warn(0x0fb721, "", "pmu io is NULL!\n");
        return AIC_EIO;
    }

    if (!pinfo->current_msk) {
        aicx_warn(0x0c9330, "", "illegal mask value!\n");
        return AIC_EINVAL;
    }

    ret = pio->pmu_read(pinfo->current_reg, &val);
    if(ret) {
        aicx_warn(0x18e916, "", "pmu io read fail!\n");
        return AIC_EIO;
    }

    val &= pinfo->current_msk;
    val >>= aic_find_first_bit(pinfo->current_msk);

    if (pinfo->current_table) {
        if (val >= current_steps)
            return AIC_EIO;

        *current_uA = pinfo->current_table[val];
        return AIC_SUCCESS;
    }

    return AIC_EIO;
}

static int pmu_sel2voltage(struct pmu_unit *punit, int sel)
{
    const struct pmu_info *pinfo = punit->pinfo;
    int max_step_nums;

    max_step_nums = (pinfo->voltage_max_uV - pinfo->voltage_min_uV)
                    / pinfo->voltage_step_uV;

    if (sel > max_step_nums) {
        aicx_warn(0x1b30dc, "", "pmu voltage target sel overflow!\n");
        return AIC_EIO;
    }

    return pinfo->voltage_min_uV + sel * pinfo->voltage_step_uV;
}

int pmu_kit_set_voltage_linear(struct pmu_unit *punit, u32 voltage_uV,
                                      u32 tolerance_uV)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    int ret, sel, final_vol;
    u32 vol_delta;

    pmu_dbg("%s %d %s: %s\n", __FILE__, __LINE__, __func__, pinfo->unit_name);

    if (pio == NULL) {
        aicx_warn(0x0fb721, "", "pmu io is NULL!\n");
        return AIC_EIO;
    }

    vol_delta = voltage_uV - pinfo->voltage_min_uV;

    if (pinfo->voltage_step_uV == 0) {
        ret = AIC_EIO;
        goto out;
    }

    if (voltage_uV < pinfo->voltage_min_uV)
        voltage_uV = pinfo->voltage_min_uV;

    sel = DIV_ROUND_UP(vol_delta, pinfo->voltage_step_uV);
    if (sel < 0) {
        ret = sel;
        goto out;
    }

    /*insure final output voltage come within tolerance*/
    final_vol = pmu_sel2voltage(punit, sel);
    if (final_vol < 0)
        return AIC_EIO;

    if (final_vol < (voltage_uV - tolerance_uV)
        || final_vol > (voltage_uV + tolerance_uV)) {
        aicx_warn(0x0fba44, "", "voltage set failed: out of tolerance!\n", __func__);
        return AIC_EIO;
    }

    ret = pmu_io_update(pio, pinfo->voltage_reg, pinfo->voltage_msk, sel);
    if (ret) {
        aicx_warn(0x0ea0f0, "s", "%s pmu write voltage fail!\n", __func__);
        goto out;
    }

    ret = pmu_io_update(pio, pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        aicx_warn(0x1a9029, "s", "%s pmu update config fail!\n", __func__);

    aic_udelay(pinfo->scaling_time_us);
out:
    return ret;
}

int pmu_kit_get_voltage_linear(struct pmu_unit *punit, u32 *voltage_uV)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    int ret;
    u32 val;

    pmu_dbg("%s %d %s: %s\n", __FILE__, __LINE__, __func__, pinfo->unit_name);

    if (pio == NULL) {
        aicx_warn(0x0fb721, "", "pmu io is NULL!\n");
        return AIC_EIO;
    }

    if (!pinfo->voltage_msk) {
        aicx_warn(0x0c9330, "", "illegal mask value!\n");
        return AIC_EINVAL;
    }

    ret = pio->pmu_read(pinfo->voltage_reg, &val);
    if(ret) {
        aicx_warn(0x18e916, "", "pmu io read fail!\n");
        return AIC_EIO;
    }

    val &= pinfo->voltage_msk;
    val >>= aic_find_first_bit(pinfo->voltage_msk);

    *voltage_uV = pinfo->voltage_min_uV + val * pinfo->voltage_step_uV;

    return AIC_SUCCESS;
}

int pmu_kit_is_enabled(struct pmu_unit *punit)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    int ret;
    u32 val;

    if (pio == NULL) {
        aicx_warn(0x0fb721, "", "pmu io is NULL!\n");
        return AIC_EIO;
    }

    ret = pio->pmu_read(pinfo->enable_reg, &val);
    if(ret) {
        aicx_warn(0x18e916, "", "pmu io read fail!\n");
        return AIC_EIO;
    }

    val &= pinfo->enable_msk;

    if (pinfo->enalbe_bit_inverted)
        return val != pinfo->enable_msk;
    else
        return val == pinfo->enable_msk;
}

int pmu_kit_enable(struct pmu_unit *punit)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    int ret;
    u32 val;

    pmu_dbg("%s %d %s: %s\n", __FILE__, __LINE__, __func__, pinfo->unit_name);

    if (pio == NULL) {
        aicx_warn(0x0fb721, "", "pmu io is NULL!\n");
        return AIC_EIO;
    }

    if (pinfo->enalbe_bit_inverted)
        val = 0;
    else
        val = pinfo->enable_msk;

    ret = pmu_io_update(pio, pinfo->enable_reg, pinfo->enable_msk, val);
    if(ret) {
        aicx_warn(0x013029, "s", "%s pmu io update fail!\n", __func__);
        return ret;
    }

    ret = pmu_io_update(pio, pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        aicx_warn(0x1a9029, "s", "%s pmu update config fail!\n", __func__);
    else
        aic_udelay(pinfo->pd_time_us);

    return ret;
}

int pmu_kit_disable(struct pmu_unit *punit)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    int ret, val;

    pmu_dbg("%s %d %s: %s\n", __FILE__, __LINE__, __func__, pinfo->unit_name);

    if (pio == NULL) {
        aicx_warn(0x0fb721, "", "pmu io is NULL!\n");
        return AIC_EIO;
    }

    if (pinfo->enalbe_bit_inverted)
        val = pinfo->enable_msk;
    else
        val = 0;

    ret = pmu_io_update(pio, pinfo->enable_reg, pinfo->enable_msk, val);
    if(ret) {
        aicx_warn(0x013029, "s", "%s pmu io update fail!\n", __func__);
        return ret;
    }

    ret = pmu_io_update(pio, pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        aicx_warn(0x1a9029, "s", "%s pmu update config fail!\n", __func__);
    else
        aic_udelay(pinfo->pd_time_us);

    return ret;
}

int pmu_kit_mulit_bit_enable(struct pmu_unit *punit)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    int ret;
    u32 val;

    pmu_dbg("%s %d %s: %s\n", __FILE__, __LINE__, __func__, pinfo->unit_name);

    if (pio == NULL) {
        aicx_warn(0x0fb721, "", "pmu io is NULL!\n");
        return AIC_EIO;
    }

    ret = pio->pmu_read(pinfo->enable_reg, &val);
    if(ret) {
        aicx_warn(0x18e916, "", "pmu io read fail!\n");
        return AIC_EIO;
    }

    if (pinfo->enalbe_bit_inverted)
        val &= ~pinfo->enable_msk;
    else
        val |= pinfo->enable_msk;

    ret = pio->pmu_write(pinfo->enable_reg, val);
    if(ret)
        aicx_warn(0x01108f, "", "pmu io write fail!\n");

    ret = pmu_io_update(pio, pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        aicx_warn(0x1a9029, "s", "%s pmu update config fail!\n", __func__);
    else
        aic_udelay(pinfo->pd_time_us);

    return ret;
}

int pmu_kit_mulit_bit_disable(struct pmu_unit *punit)
{
    const struct pmu_info *pinfo = punit->pinfo;
    const struct pmu_io *pio= punit->pmu_io;
    int ret;
    u32 val;

    pmu_dbg("%s %d %s: %s\n", __FILE__, __LINE__, __func__, pinfo->unit_name);

    if (pio == NULL) {
        aicx_warn(0x0fb721, "", "pmu io is NULL!\n");
        return AIC_EIO;
    }

    ret = pio->pmu_read(pinfo->enable_reg, &val);
    if(ret) {
        aicx_warn(0x18e916, "", "pmu io read fail!\n");
        return AIC_EIO;
    }

    if (pinfo->enalbe_bit_inverted)
        val |= pinfo->enable_msk;
    else
        val &= ~pinfo->enable_msk;

    ret = pio->pmu_write(pinfo->enable_reg, val);
    if(ret)
        aicx_warn(0x01108f, "", "pmu io write fail!\n");

    ret = pmu_io_update(pio, pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        aicx_warn(0x1a9029, "s", "%s pmu update config fail!\n", __func__);
    else
        aic_udelay(pinfo->pd_time_us);

    return ret;
}
