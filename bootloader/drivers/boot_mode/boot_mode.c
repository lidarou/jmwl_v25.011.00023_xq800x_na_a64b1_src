/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines aic boot mode Interface.
 */

#include <aic_errno.h>
#include <aic_io.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "boot_mode_comm.h"
#include <platform/aic_cmdline.h>
#include "console/console.h"
#include <wdt.h>

/*****************************************************************************/
/* Variable                                                                  */
/*****************************************************************************/
static enum BOOT_MOD status_of_bootmode=NORMAL_BOOT;
static u32 pwr_sw=0, pwr_off=0, pwr_on=0, pmic_trigger_reg1=0;
static BOOL is_warm_to_normal = AIC_FALSE;
static u32 pmic_shutdown_tirgger_reg = REG_GLB_UPS_SHUTDOWN_TRIGGER,
           pmic_sw_regsiter = REG_GLB_UPS_PMIC_SW_REGISTER,
           pmic_pwr_on_rst_flag = REG_GLB_UPS_PWR_ON_RST_FLAG,
           pmic_pwr_on_rst_flag_clr= REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR,
           pmic_pwr_off_flag = REG_GLB_UPS_PWR_OFF_FLAG;
/*****************************************************************************/
/* Function                                                                  */
/*****************************************************************************/
enum BOOT_MOD get_boot_mode(void)
{
    log_print("get_boot_mode 0x%x\n", status_of_bootmode);
    return status_of_bootmode;
}

BOOL is_ap_crash_mode(void)
{
    return ((pwr_sw & PMIC_SW_REG_CRASH_FLAG_MASK) == PMIC_SW_REG_APCRASH);
}


void bl_reset(void)
{
    u32 val = 0;

    val = PMIC_SW_REG_WARMRST | PMIC_SW_REG_BL_PHASE;
    if( AIC_SUCCESS == pmic_writel(pmic_sw_regsiter, val))
    {
        val=pmic_readl(pmic_sw_regsiter);
        log_print("bl_reset pmic sw value 0x%x\n", val);
    }
    wdt_reset(5);//pmic_wdt_reset(5) power off; wdt_reset() bb power off
}

void bl_reset_basic(void)
{
    u32 val = 0;

    val = PMIC_SW_REG_BASIC | PMIC_SW_REG_BL_PHASE;
    pmic_writel(pmic_sw_regsiter, val);

    log_print("bl reset to basic mode.\n");

    wdt_reset(5);
}

void bl_shutdown(void)
{
    u32 bootmode;

    if( AIC_SUCCESS == pmic_writel(pmic_sw_regsiter, PMIC_SW_REG_SHUTDOWN | PMIC_SW_REG_BL_PHASE))
    {
        bootmode = pmic_readl(pmic_sw_regsiter);
        log_print("bl_shutdown pmic sw value 0x%x\n", bootmode);
    }

    pmic_writel(pmic_shutdown_tirgger_reg, 0x1);
    while(1);
}

int pmic_status_reg_init(void)
{
    u32 pmic_id;

    pmic_id = (pmic_readl(REG_GLB_PD_CHIP_ID_VERSION) >> 16) & 0xFFFF;
    if (pmic_id == 0x201) {
        pmic_shutdown_tirgger_reg = REG_GLB_UPS_SHUTDOWN_TRIGGER,
        pmic_sw_regsiter = REG_GLB_UPS_PMIC_SW_REGISTER;
        pmic_pwr_on_rst_flag = REG_GLB_UPS_PWR_ON_RST_FLAG;
        pmic_pwr_on_rst_flag_clr= REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR;
        pmic_pwr_off_flag = REG_GLB_UPS_PWR_OFF_FLAG;
    } else if (pmic_id == 0x202){
        pmic_shutdown_tirgger_reg = 0x41C,
        pmic_pwr_on_rst_flag = 0x38;
        pmic_pwr_on_rst_flag_clr= 0x3c;
        pmic_pwr_off_flag = 0x34;
        pmic_sw_regsiter = 0x30;
    }

    return 0;
}

static int boot_mode_init(void)
{
    enum BOOT_MOD boot_mode=NORMAL_BOOT;
    u32 val_pmic_sw=0, val_pwr_off=0, val_pwr_on=0, val_pmic_clr=0;

    pmic_status_reg_init();

    val_pmic_sw = pmic_readl(pmic_sw_regsiter);
    val_pwr_on = pmic_readl(pmic_pwr_on_rst_flag);
    val_pwr_off = pmic_readl(pmic_pwr_off_flag);
    pmic_trigger_reg1 = pmic_readl(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1) & BIT_PMU_RTC_PWRKEY_SHUTDOWN_MODE_REG;
    log_print("boot_mode initial val_pmic_sw=0x%x,val_pwr_on=0x%x,val_pwr_off=0x%x,boot_mode=0x%x,trigger=0x%x\n", val_pmic_sw, val_pwr_on,val_pwr_off,boot_mode, pmic_trigger_reg1);
    log_print_r("%x-%x-%x-%x-%x\n", val_pmic_sw, val_pwr_on, val_pwr_off, boot_mode, pmic_trigger_reg1);

    /* step1: get the boot mode from pmic_sw_regsiter. */
    switch(val_pmic_sw & PMIC_SW_REG_BOOT_MODE_MASK)
    {
        case PMIC_SW_REG_BASIC:
            boot_mode = BASIC_BOOT;
            break;
        case PMIC_SW_REG_APCRASH:
        case PMIC_SW_REG_ADSPCRASH:
        case PMIC_SW_REG_CPCRASH:
        case PMIC_SW_REG_APWDG:
        case PMIC_SW_REG_CPWDG:
            if(val_pmic_sw & PMIC_SW_REG_DUMP_FLAG)
                boot_mode = SYSDUMP_BOOT;
            else
                boot_mode = WARMRST_BOOT;
            break;
        case PMIC_SW_REG_WARMRST:
            boot_mode = WARMRST_BOOT;
            break;
        case PMIC_SW_REG_AUTODL:
            boot_mode = AUTODL_BOOT;
            break;
        case PMIC_SW_REG_CHG_ALARM:
            boot_mode = ALARM_RTC_BOOT;
            break;
        case PMIC_SW_REG_GXEM:
            boot_mode = GXEM_BOOT;
            break;
        default:
            break;
    }

    /* step2, step3: normal boot, detect charge, alarm boot mode. */
    if (boot_mode == NORMAL_BOOT) {
        /* step2: detect charge. */
        switch(val_pwr_on & BIT_GLB_UPS_PWR_ON_FLAG_MASK)
        {
            case BIT_GLB_UPS_PWR_ON_FLAG_CHARGER_ON:
            case BIT_GLB_UPS_PWR_ON_FLAG_CHARGER_IN:
            case BIT_GLB_UPS_PWR_ON_FLAG_CHARGER_IN|BIT_GLB_UPS_PWR_ON_FLAG_CHARGER_ON:
                boot_mode = CHARGE_BOOT;
                break;

            case BIT_GLB_UPS_PWR_ON_FLAG_UPS_ALARM:
                boot_mode = ALARM_UPS_BOOT;
                break;
            case BIT_GLB_UPS_PWR_ON_FLAG_RTC_ALARM:
                boot_mode = ALARM_RTC_BOOT;
                break;
            default:
                break;
        }

        /* step3: detect alarm. */
        if (val_pwr_off & BIT_GLB_UPS_PWR_FLAG_RSTIN_N)//rst key
        {
            if(val_pwr_on != BIT_GLB_UPS_PWR_ON_FLAG_RST_REQ)
                boot_mode = WARMRST_BOOT;//poweroff + reset key
        }

        /* check 12s rest. */
        if (val_pwr_off & BIT_GLB_UPS_PWR_FLAG_SOFTKEY)//pwr_on=0x0; pwr_off=0x100 12s reset
        {
            if (!pmic_trigger_reg1)
                boot_mode = WARMRST_BOOT;
        }
    }

    /* step4: reject shutdown normally, not warmreset. */
    if (pmic_trigger_reg1) //HARD RESET 1
    {
        if(val_pwr_off & BIT_GLB_UPS_PWR_OFF_SHUTDOWN_PON_KEY_HARD_MASK)
        {
            if(boot_mode == WARMRST_BOOT)
            {
                boot_mode = NORMAL_BOOT;
                is_warm_to_normal = AIC_TRUE;
            }
        }
    }
    else
    {
        if(val_pwr_off & BIT_GLB_UPS_PWR_OFF_SHUTDOWN_PON_KEY_SOFT_MASK)
        {
            if(boot_mode == WARMRST_BOOT)
            {
                boot_mode = NORMAL_BOOT;
                is_warm_to_normal = AIC_TRUE;
            }
        }
    }

    if ((boot_mode == CHARGE_BOOT) && (val_pwr_off & BIT_GLB_UPS_PWR_OFF_FLAG_KEY))
        boot_mode = NORMAL_BOOT;

/* connect with extern ap, long power key, just reboot. */
#ifndef CONFIG_CONN_WITH_EXTERN_AP
#if !CONFIG_USER_VERSION
    u32 pmic_id, lpr_pwr_flag = 0x100;
    pmic_id = (pmic_readl(REG_GLB_PD_CHIP_ID_VERSION) >> 16) & 0xffff;
    if (pmic_id == 0x202)
        lpr_pwr_flag = 0x200;
    else if (pmic_id == 0x201)
        lpr_pwr_flag = 0x100;

    if ((val_pwr_off & lpr_pwr_flag) && !(val_pmic_sw & BIT_15)
        && (val_pmic_sw & PMIC_SW_REG_DUMP_FLAG))
        boot_mode = LP_SYSDUMP_BOOT;
#endif
#endif

    status_of_bootmode = boot_mode;
    pwr_sw = val_pmic_sw;
    pwr_off = val_pwr_off;
    pwr_on = val_pwr_on;
    //clear register
    pmic_writel(pmic_sw_regsiter,0x0);
    val_pmic_clr = 0xF;//clr all status
    pmic_writel(pmic_pwr_on_rst_flag_clr,val_pmic_clr);
    val_pmic_sw = pmic_readl(pmic_sw_regsiter);
    val_pwr_on = pmic_readl(pmic_pwr_on_rst_flag);
    val_pwr_off = pmic_readl(pmic_pwr_off_flag);
    log_print("boot_mode clr val_pmic_sw=0x%x,val_pmic_on=0x%x,val_pwr_off=0x%x,boot_mode=0x%x\n", val_pmic_sw, val_pwr_on, val_pwr_off,boot_mode);

    return 0;
}
core_init(boot_mode_init);

#if CONFIG_CMDLINE
static char *get_boot_mode_str(void)
{
    char *str;

    switch (status_of_bootmode) {
        case NORMAL_BOOT:
        case WARMRST_BOOT:
        str = "nrm";
        break;

        case ALARM_UPS_BOOT:
        str = "ups";
        break;

        case ALARM_RTC_BOOT:
        str = "rtc";
        break;

        case BASIC_BOOT:
        str = "bsc";
        break;

        case CHARGE_BOOT:
        str = "chg";
        break;

        case GXEM_BOOT:
        str = "gxm";
        break;

        case FOTA_BOOT:
        case SYSDUMP_BOOT:
        case DOWNLOAD_BOOT:
        case AUTODL_BOOT:
        case LP_SYSDUMP_BOOT:
        default:
            str = "err";
            break;
    }

    return str;
}
#endif

static int boot_mode_transfer(void)
{
#if CONFIG_CMDLINE
#if CONFIG_USER_VERSION
    aic_cmdline_add_key_int(cl_key_usermode,1);
#else
    aic_cmdline_add_key_int(cl_key_usermode,0);
#endif
    aic_cmdline_add_key(cl_key_bootmode, get_boot_mode_str());
    aic_cmdline_add_key_int(cl_key_pwr_sw,pwr_sw);
    aic_cmdline_add_key_int(cl_key_pwr_on,pwr_on);
    aic_cmdline_add_key_int(cl_key_pwr_off,pwr_off);
    aic_cmdline_add_assert_info();
#endif
    return 0;
}
early_init(boot_mode_transfer);
