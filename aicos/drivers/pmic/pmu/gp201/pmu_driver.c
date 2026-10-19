/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  gp201 pmu driver file.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1953f5
#define LOG_MODULE    AMOD_PMIC

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <pmu_driver.h>
#include <pmu_kit.h>
#include <pmic_efuse.h>

#define GP201_SCALING_SETTLING_TIME 10 //us
#define GP201_PD_SETTLING_TIME 2000 //us

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define RECONFIG_PMU_NUMS   3
#define GP201_SUB_VERSION   0x5
#define GP201_CHOPPER_DIRECT_BLK 0
#define GP201_CHOPPER_DIRECT_BIT 24
#define GP201_CHOPPER_FLIP_REG  0X80f4

char *reconfig_pmu_list[RECONFIG_PMU_NUMS] = {"DVDD3V", "VDDBK3V3", "VDDSD3V"};

int gp201_pmu_write(u32 reg, u32 val)
{
    return pmic_writel((u16) reg, val);
}

int gp201_pmu_read(u32 reg, u32 *val)
{
    int ret;

    ret = pmic_burst_read((u16) reg, (u8 *)val, 4);

    return ret;
}

static struct pmu_ops gp201_pmu_ops = {
        .enable = pmu_kit_enable,
        .disable = pmu_kit_disable,
        .is_enabled = pmu_kit_is_enabled,
        .get_voltage = pmu_kit_get_voltage_linear,
        .set_voltage = pmu_kit_set_voltage_linear,
};

static struct pmu_io gp201_pmu_io = {
        .pmu_read = gp201_pmu_read,
        .pmu_write = gp201_pmu_write,
};

#define PMU_UNIT_INFO(_unit_name, _parent_name, vol_reg, vol_msk, en_reg, en_msk,\
                     update_reg, update_msk, vol_min_uV, vol_max_uV, vol_step_uV,\
                    scaling_settling_us, pd_settling_us, punit_always_on, \
                    punit_boot_on, punit_bypass, punit_scaling_boot_voltage) {          \
         .unit_name            = #_unit_name,        \
         .parent_name          = #_parent_name,      \
         .voltage_reg          = vol_reg,      \
         .voltage_msk          = vol_msk,      \
         .enable_reg           = en_reg,       \
         .enable_msk           = en_msk,       \
         .cfg_update_reg       = update_reg,   \
         .cfg_update_msk       = update_msk,   \
         .voltage_min_uV       = vol_min_uV,   \
         .voltage_max_uV       = vol_max_uV,   \
         .voltage_step_uV      = vol_step_uV,  \
         .scaling_time_us      = scaling_settling_us, \
         .pd_time_us           = pd_settling_us, \
         .scaling_boot_voltage = punit_scaling_boot_voltage, \
         .always_on            = punit_always_on,        \
         .boot_on              = punit_boot_on,          \
         .bypass               = punit_bypass,  \
         .pops                 = &gp201_pmu_ops,   \
         .enalbe_bit_inverted  = true,   \
    }

static struct pmu_info pmu_units_info[] = {
    #include <pmic_pmu_units.h>
};

int gp201_pmu_init(void)
{
    int ret, i, j, result = 0, pmic_id = 0;
    bool need_compatible = true;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    aicx_warn(0x064fb2, "x", "pmic_id:0x%x\n", pmic_id);
    if (BIT_GLB_PD_CHIP_ID_SUB_VERSION(pmic_id) < GP201_SUB_VERSION)
        need_compatible = false;

    for (i = 0; i < ARRAY_SIZE(pmu_units_info); i++) {
        if (need_compatible)
        for(j = 0; j < RECONFIG_PMU_NUMS; j++ ) {
            if (!strcmp(pmu_units_info[i].unit_name, reconfig_pmu_list[j])) {
                pmu_units_info[i].enalbe_bit_inverted = false;
                break;
            }
        }

        ret = pmu_register(&pmu_units_info[i], &gp201_pmu_io);
        if (ret)
            aicx_warn(0x0a1bd8, "ss", "%s pmu unit %s register fail!\n", __func__, pmu_units_info[i].unit_name);
        result |= ret;
    }

    return result;
}

/*gp202 operate info*/

/*PMIC_BASE_GLB_PD same between GP201 GP202*/
#define REG_GLB_PD_BUCK0_ENABLE_REG                                             (PMIC_BASE_GLB_PD + 0x110)
#define REG_GLB_PD_BUCK0_CFG_REG1                                               (PMIC_BASE_GLB_PD + 0x114)
#define REG_GLB_PD_BUCK0_CFG_REG2                                               (PMIC_BASE_GLB_PD + 0x118)
#define REG_GLB_PD_BUCK0_VOUT_TRIM_REG                                          (PMIC_BASE_GLB_PD + 0x11c)
#define REG_GLB_PD_BUCK0_RESERVED_BIT_REG                                       (PMIC_BASE_GLB_PD + 0x120)
#define REG_GLB_PD_BUCK1_ENABLE_REG                                             (PMIC_BASE_GLB_PD + 0x130)
#define REG_GLB_PD_BUCK1_CFG_REG1                                               (PMIC_BASE_GLB_PD + 0x134)
#define REG_GLB_PD_BUCK1_CFG_REG2                                               (PMIC_BASE_GLB_PD + 0x138)
#define REG_GLB_PD_BUCK1_RESERVED_BIT_REG                                       (PMIC_BASE_GLB_PD + 0x13c)
#define REG_GLB_PD_LDO0_EN_REG                                                  (PMIC_BASE_GLB_PD + 0x150)
#define REG_GLB_PD_LDO0_CFG_REG                                                 (PMIC_BASE_GLB_PD + 0x154)
#define REG_GLB_PD_LDO1_EN_REG                                                  (PMIC_BASE_GLB_PD + 0x158)
#define REG_GLB_PD_LDO1_CFG_REG                                                 (PMIC_BASE_GLB_PD + 0x15c)
#define REG_GLB_PD_LDO2_EN_REG                                                  (PMIC_BASE_GLB_PD + 0x160)
#define REG_GLB_PD_LDO2_CFG_REG                                                 (PMIC_BASE_GLB_PD + 0x164)
#define REG_GLB_PD_LDO3_EN_REG                                                  (PMIC_BASE_GLB_PD + 0x168)
#define REG_GLB_PD_LDO3_CFG_REG                                                 (PMIC_BASE_GLB_PD + 0x16c)
#define REG_GLB_PD_LDO4_EN_REG                                                  (PMIC_BASE_GLB_PD + 0x170)
#define REG_GLB_PD_LDO4_CFG_REG                                                 (PMIC_BASE_GLB_PD + 0x174)
#define REG_GLB_PD_LDO5_EN_REG                                                  (PMIC_BASE_GLB_PD + 0x178)
#define REG_GLB_PD_LDO5_CFG_REG                                                 (PMIC_BASE_GLB_PD + 0x17c)
#define REG_GLB_PD_LDO6_EN_REG                                                  (PMIC_BASE_GLB_PD + 0x180)
#define REG_GLB_PD_LDO6_CFG_REG                                                 (PMIC_BASE_GLB_PD + 0x184)

#define BIT_GLB_PD_BUCK0_VOUT_TRIM_NORM_REG(x)                                  (((x) & 0x3f))
#define BIT_GLB_PD_BUCK1_VOUT_TRIM_REG(x)                                       (((x) & 0x3f) << 16)
#define BIT_GLB_PD_LDO_VSEL_REG(x, y)                                           (((x) & y) << 20)

#define GP202_BASE_PMU_RTC 0x4200
#define REG_PMU_RTC_XTL_EN_REG                                                  (GP202_BASE_PMU_RTC + 0x20)
#define REG_PMU_RTC_SLEEP_EN_REG                                                (GP202_BASE_PMU_RTC + 0x24)
#define REG_PMU_RTC_BUCK0_BOUND_REG                                             (GP202_BASE_PMU_RTC + 0x28)
#define REG_PMU_RTC_BUCK1_BOUND_REG                                             (GP202_BASE_PMU_RTC + 0x2c)
#define REG_PMU_RTC_LDO0_BOUND_REG                                              (GP202_BASE_PMU_RTC + 0x30)
#define REG_PMU_RTC_LDO1_BOUND_REG                                              (GP202_BASE_PMU_RTC + 0x34)
#define REG_PMU_RTC_LDO2_BOUND_REG                                              (GP202_BASE_PMU_RTC + 0x38)
#define REG_PMU_RTC_LDO3_BOUND_REG                                              (GP202_BASE_PMU_RTC + 0x3c)
#define REG_PMU_RTC_LDO4_BOUND_REG                                              (GP202_BASE_PMU_RTC + 0x40)
#define REG_PMU_RTC_LDO5_BOUND_REG                                              (GP202_BASE_PMU_RTC + 0x44)
#define REG_PMU_RTC_LDO6_BOUND_REG                                              (GP202_BASE_PMU_RTC + 0x48)
#define REG_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG                                     (GP202_BASE_PMU_RTC + 0x4c)

#define BIT_PMU_RTC_BUCK_LDO_POWER_DOWN_REG                                     BIT(0)
#define BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG                                     BIT(0)

#define PMU_UNIT_INFO_OPS(_unit_name, _parent_name, vol_reg, vol_msk, en_reg, en_msk,\
                     update_reg, update_msk, vol_min_uV, vol_max_uV, vol_step_uV,\
                    scaling_settling_us, pd_settling_us, punit_always_on, \
                    punit_boot_on, punit_bypass, pmu_ops, punit_scaling_boot_voltage) {\
         .unit_name            = #_unit_name,        \
         .parent_name          = #_parent_name,      \
         .voltage_reg          = vol_reg,      \
         .voltage_msk          = vol_msk,      \
         .enable_reg           = en_reg,       \
         .enable_msk           = en_msk,       \
         .cfg_update_reg       = update_reg,   \
         .cfg_update_msk       = update_msk,   \
         .voltage_min_uV       = vol_min_uV,   \
         .voltage_max_uV       = vol_max_uV,   \
         .voltage_step_uV      = vol_step_uV,  \
         .scaling_time_us      = scaling_settling_us, \
         .pd_time_us           = pd_settling_us, \
         .scaling_boot_voltage = punit_scaling_boot_voltage, \
         .always_on            = punit_always_on,        \
         .boot_on              = punit_boot_on,          \
         .bypass               = punit_bypass,  \
         .pops                 = pmu_ops,   \
         .enalbe_bit_inverted  = true,   \
    }

#ifndef CONFIG_SUBSYS_AP
int gp202_ldo0_1_set_voltage(struct pmu_unit *punit, u32 voltage_uV,
                                      u32 tolerance_uV)
{
    struct pmu_info *pinfo = punit->pinfo;
    struct pmu_io *pio= punit->pmu_io;
    int ret;
    u32 voltage_base = pinfo->voltage_min_uV;

    if (voltage_uV > pinfo->voltage_max_uV) {
        ret = pmu_io_update(pio, pinfo->voltage_reg, 0x1, 0x1);
        pinfo->voltage_min_uV += 600000;
    } else {
        ret = pmu_io_update(pio, pinfo->voltage_reg, 0x1, 0x0);
    }

    if (ret) {
        aicx_warn(0x10a379, "s", "%s pmu switch power range fail!\n", __func__);
        goto out;
    }

    ret = pmu_kit_set_voltage_linear(punit, voltage_uV, tolerance_uV);
    if (ret)
        aicx_warn(0x083906, "s", "%s pmu scaling voltage fail!\n", __func__);
out:
    pinfo->voltage_min_uV = voltage_base;
    return ret;
}

int gp202_ldo0_1_get_voltage(struct pmu_unit *punit, u32 *voltage_uV)
{
    struct pmu_info *pinfo = punit->pinfo;
    struct pmu_io *pio= punit->pmu_io;
    u32 voltage_base = pinfo->voltage_min_uV;
    int ret;
    u32 val;

    ret= pio->pmu_read(pinfo->voltage_reg, &val);
    if (ret) {
        aicx_warn(0x050912, "s", "%s get power range fail!\n", __func__);
        goto out;
    }

    if (val & 0x1)
        pinfo->voltage_min_uV += 600000;

    ret = pmu_kit_get_voltage_linear(punit, voltage_uV);
    if (ret)
        aicx_warn(0x15d79f, "s", "%s pmu get voltage fail!\n", __func__);

out:
    pinfo->voltage_min_uV = voltage_base;
    return ret;
}

static const struct pmu_ops gp202_pmu_ldo0_1_ops = {
        .enable = pmu_kit_enable,
        .disable = pmu_kit_disable,
        .is_enabled = pmu_kit_is_enabled,
        .get_voltage = gp202_ldo0_1_get_voltage,
        .set_voltage = gp202_ldo0_1_set_voltage,
};
#endif

int gp202_ldo_gen_set_voltage(struct pmu_unit *punit, u32 voltage_uV,
                                      u32 tolerance_uV)
{
    struct pmu_info *pinfo = punit->pinfo;
    struct pmu_io *pio= punit->pmu_io;
    int ret, sel;

    if (voltage_uV > 3100000) {
        pinfo->voltage_min_uV = 3100000;
        sel = 0x3;
    } else if (voltage_uV > 2800000) {
        pinfo->voltage_min_uV = 2800000;
        sel = 0x2;
    } else if (voltage_uV > 2600000) {
        pinfo->voltage_min_uV = 2600000;
        sel = 0x1;
    } else {
        pinfo->voltage_min_uV = 1600000;
        sel = 0;
    }

    ret = pmu_io_update(pio, pinfo->voltage_reg, 0x3 << 25, sel);
    if (ret) {
        aicx_warn(0x10a379, "s", "%s pmu switch power range fail!\n", __func__);
        goto out;
    }

    ret = pmu_kit_set_voltage_linear(punit, voltage_uV, tolerance_uV);
    if (ret)
        aicx_warn(0x083906, "s", "%s pmu scaling voltage fail!\n", __func__);
out:
    return ret;
}

int gp202_ldo_gen_get_voltage(struct pmu_unit *punit, u32 *voltage_uV)
{
    struct pmu_info *pinfo = punit->pinfo;
    struct pmu_io *pio= punit->pmu_io;
    int ret, sel;
    u32 val;

    ret= pio->pmu_read(pinfo->voltage_reg, &val);
    if (ret) {
        aicx_warn(0x050912, "s", "%s get power range fail!\n", __func__);
        goto out;
    }

    sel = (val >> 25) & 0x3;

    switch (sel) {
    case 0x3:
        pinfo->voltage_min_uV = 3100000;
        break;
    case 0x2:
        pinfo->voltage_min_uV = 2800000;
        break;
    case 0x1:
        pinfo->voltage_min_uV = 2600000;
        break;
    case 0x0:
    default:
        pinfo->voltage_min_uV = 1600000;
    }

    ret = pmu_kit_get_voltage_linear(punit, voltage_uV);
    if (ret)
        aicx_warn(0x15d79f, "s", "%s pmu get voltage fail!\n", __func__);

out:
    return ret;
}

static const struct pmu_ops gp202_pmu_ldo_gen_ops = {
        .enable = pmu_kit_enable,
        .disable = pmu_kit_disable,
        .is_enabled = pmu_kit_is_enabled,
        .get_voltage = gp202_ldo_gen_get_voltage,
        .set_voltage = gp202_ldo_gen_set_voltage,
};

struct pmu_info gp202_pmu_units_info[] = {
    #include <gp202_pmu_units.h>
};

int gp202_pmu_init(void)
{
    int ret, i, result = 0;

    for (i = 0; i < ARRAY_SIZE(gp202_pmu_units_info); i++) {
        ret = pmu_register(&gp202_pmu_units_info[i], &gp201_pmu_io);
        if (ret)
            aicx_warn(0x0a1bd8, "ss", "%s pmu unit %s register fail!\n", __func__, gp202_pmu_units_info[i].unit_name);
        result |= ret;
    }

    return result;
}

int pmic_pmu_init(void)
{
    int ret =0, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202)
        ret = gp202_pmu_init();
    else
        ret = gp201_pmu_init();

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(pmic_pmu_init);

#ifdef CONFIG_SUBSYS_AP
int gp201_version_compatible(void)
{
    int ret =0, pmic_id;
    u32 data;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202)
        goto out;

    if (BIT_GLB_PD_CHIP_ID_SUB_VERSION(pmic_id) < GP201_SUB_VERSION)
        goto out;

    ret = aic_pmic_efuse_block_read(GP201_CHOPPER_DIRECT_BLK, &data);
    if (ret) {
        aicx_warn(0x1a443c, "sd", "%s read pmic efuse blk:%d failed!", __func__, GP201_CHOPPER_DIRECT_BLK);
        goto out;
    }

    if (data & GP201_CHOPPER_DIRECT_BIT) {
        pmic_writel(GP201_CHOPPER_FLIP_REG, pmic_readl(GP201_CHOPPER_FLIP_REG) | BIT(0));
    }

out:
    return ret;
}
AIC_DRV_L1_INIT_EXPORT(gp201_version_compatible);

#ifdef CONFIG_MHA_PA_POWER
#include <rfpa_vbat_supply_list.h>
#include <aic_at_process_common.h>

static aos_thread_id notice_cp_thread = AOS_INVALID_THREAD_ID;
#define NOTICE_CP_TASK_PRIO   5
#define NOTICE_CP_MAX_CNT     10

static void notice_cp_thread_entry(void *data)
{
    int notice_cnt = 0;
    AT_RESP_FROM_CP at_cp_resp = {0};

    /* malloc */
    at_cp_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == at_cp_resp.resp_buf) {
        aicx_error(0x0e3ec0, "s", "[%s] malloc failed!\r\n", __func__);
        return;
    }
    at_cp_resp.buf_len = AT_MAX_AP_RESP_LENGTH;

    while(1) {
        if (notice_cnt == NOTICE_CP_MAX_CNT) {
            aicx_error(0x1c8182, "", "notice cp switch rfpa supply failed!!!\n");
            break;
        }

        /* send AT+PABUCK2=0 command,respnse: */
        aic_send_cmd_and_wait("AT+PABUCK2=0", &at_cp_resp);

        if(strstr(at_cp_resp.resp_buf, "OK")) {
            aicx_warn(0x078989, "s", "[%s] rfpa supply adapt success!\r\n", __func__);
            break;
        }

        notice_cnt++;
        aos_sleep(1000);
    }

    aos_free(at_cp_resp.resp_buf);
    at_cp_resp.resp_buf = NULL;
}

int notice_cp_pa_supply(void)
{
    notice_cp_thread = aos_create_thread("notice_cp_thread", notice_cp_thread_entry, AOS_NULL, 1024, 8,
                      "notice_cp_thread", NOTICE_CP_TASK_PRIO, AOS_AUTO_START);
    if (notice_cp_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x146d13, "s", "%s create task fail!\n", __func__);
        return AIC_ENXIO;
    }

    return 0;
}

int rfpa_power_adapt(void)
{
    bool vbat_supply_pa = true;
    int board_id = 0, i, ret = 0;

    board_id = aic_cmdline_get_value_int(cl_key_boardid);

    for (i = 0; i < sizeof(rfpa_buck2_supply_list)/sizeof(rfpa_buck2_supply_list[0]); i++) {
        if (board_id == rfpa_buck2_supply_list[i]) {
            vbat_supply_pa = false;
            break;
        }
    }

    if (vbat_supply_pa)
        ret = notice_cp_pa_supply();

    return ret;
}
AIC_DRV_L1_INIT_EXPORT(rfpa_power_adapt);
#endif
#endif
