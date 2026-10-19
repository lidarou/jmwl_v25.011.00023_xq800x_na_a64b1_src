/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of sysoff module.
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16c831
#define LOG_MODULE    AMOD_DRV

#include "sysoff_manage.h"
#include <adbg_cfg_api.h>
#include <aic_boot_mode.h>
#include <cache.h>
#include <core_op.h>
#include <sys_mem_api.h>
#include <sysoff.h>
#include <reg_base.h>
#include <top_glb.h>
#include <aon_glb.h>
#include <aic_init.h>
#include <pmic_io.h>
#include <pmic_auto_reg.h>

#include <wdt.h>
#include <aic_cmdline.h>
#include <aic_at_interact_cp.h>
#include <assert_adpter.h>
#include <subsys_api.h>
#include <busmon_inc.h>
#include <dmc_mpu_inc.h>
#ifdef CONFIG_AIC_CTRL_SUPPORT
#include <aic_ctrl.h>
#endif

static LIST_HEAD(s_sysoff_hook_info);

static u16 pmic_shutdown_tirgger_reg = 0x200,
           pmic_sw_regsiter = 0x204;

//deepshutdown command
static u16 pmic_deepsd_trigger_reg= 0x450;  // 1 second
static u16 pmic_deepsd_ups_pu_reg =0xdc; //0 first

uint32_t sysoff_flag = 0;
SYSOFF_MODE_MSG mode_crash = SYSOFF_MODE_MAX;
static int cnt_info = 0;
static void sysoff_mode(SYSOFF_MODE_MSG sysoff_mode);
static void sysoff_handler(void);

u32 sysoff_register_hook(SYSOFF_HOOK_FUNC      sysoff_func,
                            void*           sysoff_param_ptr)
{
    SYSOFF_HOOK_T *sysoff_hook;

    if (!sysoff_func)
        return AIC_EINVAL;

    sysoff_hook = aos_zalloc(sizeof(SYSOFF_HOOK_T));
    if (!sysoff_hook) {
        return AIC_ENODEV;
    }
    list_head_init(&sysoff_hook->link);
    sysoff_hook->sysoff_func = sysoff_func;
    sysoff_hook->sysoff_param_ptr = sysoff_param_ptr;

    /* 2.add lock to hook list */
    list_add(&sysoff_hook->link, &s_sysoff_hook_info);

    cnt_info++;
    return (u32)sysoff_hook;
}

int sysoff_unregister_hook(u32 handle)
{
    SYSOFF_HOOK_T *sysoff_hook;

    if(!handle)
        return AIC_EINVAL;

    sysoff_hook = (SYSOFF_HOOK_T *)handle;

    /* del lock from hook list */
    list_del(&sysoff_hook->link);
    aos_free(handle);
    return AIC_SUCCESS;
}

void sysoff_common_handle(SYSOFF_MODE_MSG mode, bool is_handler, bool is_cache, bool is_sleep, u32 sleep_time )
{
    sysoff_mode(mode);

    if(is_handler)
        sysoff_handler();

    if(is_sleep)
        aos_sleep(sleep_time);

    /* flush dcache */
    if(is_cache)
        dcache_invcc_all();

    wdt_reset(5);
}

void sysoff_sysdump()
{
    if(mode_crash == SYSOFF_MODE_MAX)
        sysoff_mode(SYSOFF_MODE_APCRASH);

    /* flush dcache */
    dcache_invcc_all();

    wdt_reset(5);
}

void sysoff_reset()
{
    sysoff_common_handle(SYSOFF_MODE_WARMRESET, AIC_TRUE, AIC_TRUE, AIC_TRUE, 1500);
}

void sysoff_emergency_reset()
{
    sysoff_common_handle(SYSOFF_MODE_WARMRESET, AIC_FALSE, AIC_TRUE, AIC_FALSE, 0);
}

void sysoff_chgalrm_reset(void)
{
    sysoff_common_handle(SYSOFF_MODE_CHG_ALARM, AIC_FALSE, AIC_TRUE, AIC_FALSE, 0);
}

void sysoff_gxem_reset(void)
{
    sysoff_common_handle(SYSOFF_MODE_GXEM, AIC_FALSE, AIC_TRUE, AIC_FALSE, 0);
}

//customer qpowd mode: 0, directory shutdown
void sysoff_emergency_shutdown()
{
    u32 val = 0x1;

    /* switch to hw cali for shutdown cali */
    val = pmic_readl(0x54);
    val &= ~BIT(12);
    pmic_writel(0x54, val);

    val = pmic_readl(0x58);
    val &= ~BIT(12);
    pmic_writel(0x58, val);

    val = pmic_readl(0x23c);
    val |= BIT(2);
    pmic_writel(0x23c, val);

    sysoff_mode(SYSOFF_MODE_SHUTDOWN);
    pmic_writel(pmic_shutdown_tirgger_reg, 1);
    while(1);
}

void sysoff_shutdown()
{
    sysoff_handler();

    /* flush dcache */
    dcache_invcc_all();

#ifdef CONFIG_AIC_CTRL_SUPPORT
    aic_ctrl_state_notify(AIC_SHUTDOWN_STATE);
    aos_sleep(1000);
#endif

    sysoff_emergency_shutdown();
}

void sysoff_deepshutdw()
{
    int pmic_id;
    sysoff_handler();

    /* flush dcache */
    dcache_invcc_all();

#ifdef CONFIG_AIC_CTRL_SUPPORT
    aic_ctrl_state_notify(AIC_SHUTDOWN_STATE);
    aos_sleep(1000);
#endif

    sysoff_mode(SYSOFF_MODE_DPSHUTDW);
    pmic_id = (aic_cmdline_get_value_int(cl_key_pmic_version) >> 16) & 0xFFFF;
    if (pmic_id == 0x202) {
        pmic_writel(pmic_deepsd_ups_pu_reg, 0x0);
        pmic_writel(pmic_deepsd_trigger_reg, 0x1);
        while(1);
    }
}


void sysoff_reset_basic(void)
{
    sysoff_common_handle(SYSOFF_MODE_BASIC, AIC_TRUE, AIC_TRUE, AIC_TRUE, 1500);
}

void sysoff_stop_cp(void)
{
    if (SUBSYS_STATE_POWER_OFF == subsys_get_state(AIC_SYS_CP))
        return;

    /* send at+cfun = 0 to cp. */
    aic_send_cfun_cmd(false);

    /* assert cp */
    ap_notify_other_sys_assert(AIC_SYS_CP);

    /* power off cp */
    subsys_power_off(AIC_SYS_CP);

    aos_sleep(100);

    /* disable the busmonitor. */
    if (!aic_boot_mode_is_basic()) {
        gx_monitor_suspend();
        gx_mpu_mon_suspend();
    }

    /* mpu config cp region. */
    subsys_mpu_config_sys(AIC_SYS_CP);

    /* create the ext heap. */
    sys_create_basic_ext_heap();
}

void sysoff_autodl()
{
    sysoff_common_handle(SYSOFF_MODE_AUTODL, AIC_TRUE, AIC_FALSE, AIC_FALSE, 0);
}

static int sysoff_save_mode(u32 mode, boolean is_freq_point)
{
    u32 pmic_val,freq_point;
    int ret;

    if(!is_freq_point)
    {
        pmic_val = pmic_readl(pmic_sw_regsiter);
        pmic_val = (pmic_val & (PMIC_SW_MASK | PMIC_SW_REG_DUMPMODE_FLAG_MASK)) | mode;
    }
    else
    {
        freq_point = mode << PMIC_SW_MASK_SHIFT_BIT;//mode bit14/bit15
        pmic_val = pmic_readl(pmic_sw_regsiter) & (~PMIC_SW_MASK);
        pmic_val |= freq_point;
    }

    if(adbg_is_ap_dump())
        AIC_SET_MASK(pmic_val, PMIC_SW_REG_DUMPMODE_FLAG_MASK);
    else
        AIC_CLR_MASK(pmic_val, PMIC_SW_REG_DUMPMODE_FLAG_MASK);

    ret = pmic_writel(pmic_sw_regsiter, pmic_val);
    if( !ret )
        sysoff_flag = pmic_val;
    else
        sysoff_flag = mode;

    return ret;
}

static u32 sysoff_mode_to_pmic_val(SYSOFF_MODE_MSG sysoff_mode)
{
    u32 val;

    switch(sysoff_mode)
    {
        case SYSOFF_MODE_APCRASH:
            val = PMIC_SW_REG_APCRASH;
            break;
        case SYSOFF_MODE_ADSPCRASH:
            val = PMIC_SW_REG_ADSPCRASH;
            break;
        case SYSOFF_MODE_CPCRASH:
            val = PMIC_SW_REG_CPCRASH;
            break;
        case SYSOFF_MODE_APWDG:
            val = PMIC_SW_REG_APWDG;
            break;
        case SYSOFF_MODE_CPWDG:
            val = PMIC_SW_REG_CPWDG;
            break;
        case SYSOFF_MODE_ADSPWDG:
            val = PMIC_SW_REG_ADSPWDG;
            break;
        case SYSOFF_MODE_BASIC:
            val = PMIC_SW_REG_BASIC;
            break;
        case SYSOFF_MODE_WARMRESET:
        case SYSOFF_MODE_COLDRESET:
            val = PMIC_SW_REG_WARMRST;
            break;
        case SYSOFF_MODE_SHUTDOWN:
            val = PMIC_SW_REG_SHUTDOWN;
            break;
        case SYSOFF_MODE_AUTODL:
            val = PMIC_SW_REG_AUTODL;
            break;
        case SYSOFF_MODE_CHG_ALARM:
            val = PMIC_SW_REG_CHG_ALARM;
            break;
        case SYSOFF_MODE_GXEM:
            val = PMIC_SW_REG_GXEM;
            break;
        case SYSOFF_MODE_DPSHUTDW:
            val = PMIC_SW_REG_DPSHUTDW;
            break;
        default:
            val = PMIC_SW_REG_BOOT_MODE_MASK;
            break;
    }

    return val;
}

int sysoff_set_dumpmode(SYSOFF_MODE_MSG sysoff_mode)
{
    u32 val = sysoff_mode_to_pmic_val(sysoff_mode);
    int ret = sysoff_save_mode(val, 0);

    if(!ret)
        mode_crash = sysoff_mode;
    return ret;
}

static void sysoff_mode(SYSOFF_MODE_MSG sysoff_mode)
{
    u32 val= sysoff_mode_to_pmic_val(sysoff_mode);

    sysoff_save_mode(val, 0);
    return;
}

static void sysoff_handler(void)
{
    SYSOFF_HOOK_T *sysoff_hook = NULL;

    if (aic_sys_is_assert())
        return;

    /* must called in task context. */
    AIC_ASSERT(aos_is_user_thread_context());

    /* call sysoff func. */
    list_for_each_entry(sysoff_hook, &s_sysoff_hook_info, link, SYSOFF_HOOK_T) {
        if(sysoff_hook->sysoff_func != NULL) {
            /* skip the extend image function call. */
            if (aic_boot_mode_is_basic() &&
                sys_is_belong_extend_image(sysoff_hook->sysoff_func))
                continue;

            sysoff_hook->sysoff_func(sysoff_hook->sysoff_param_ptr);
            cnt_info--;
        }
    }
    aicx_info(0x04a5d9, "d", "cnt_info end %d\n", cnt_info);
}

int pmic_status_reg_init(void)
{
    int pmic_id;

    pmic_id = (aic_cmdline_get_value_int(cl_key_pmic_version) >> 16) & 0xFFFF;
    if (pmic_id == 0x202) {
        pmic_shutdown_tirgger_reg = 0x41C,
        pmic_sw_regsiter = 0x30;
    } else if (pmic_id == 0x201){
        pmic_shutdown_tirgger_reg = 0x200,
        pmic_sw_regsiter = 0x204;
    }

    if (!aic_is_user_mode())
        pmic_writel(pmic_sw_regsiter, pmic_readl(pmic_sw_regsiter) | PMIC_SW_REG_DUMPMODE_FLAG_MASK);

    return 0;
}
AIC_DRV_P1_INIT_EXPORT(pmic_status_reg_init);
