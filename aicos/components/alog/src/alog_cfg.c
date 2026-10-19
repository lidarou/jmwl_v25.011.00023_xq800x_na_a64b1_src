/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log mudule config file. It contains all subsys log configurations.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10eb29
#define LOG_MODULE    AMOD_ALOG

#include <aic_size.h>
#include <aic_bitmap.h>
#include <aic_boot_mode.h>
#include <aic_cmdline.h>
#include <AIC_DEBUG.h>
#include <aic_io_dev.h>
#include <alog_inner.h>
#include <alog_cfg.h>
#include <alog_api_fast.h>
#include <alog_outputer.h>
#include <adbg_cfg_api.h>
#include <alog_trace_filter.h>

#ifdef CONFIG_NVRAM_SUPPORT
#ifdef CONFIG_MISC_SUPPORT
#include <aic_md_api.h>
#include <aic_md_id.h>

#define DBG_NV_ID       MD_ID_DEBUG
#define alog_nv_read(id, size, buf)         aic_md_read(id, size, buf)
#define alog_nv_write(id, size, buf, fs)    aic_md_write(id, size, buf)
#else
#include <aic_nvram_id.h>
#include <aic_nvram_api.h>

#define DBG_NV_ID       NVRAM_ID_DEBUG
#define alog_nv_read(id, size, buf)         aic_nvram_read(id, size, buf)
#define alog_nv_write(id, size, buf, fs)    aic_nvram_sync_write(id, size, buf, fs)
#endif /* CONFIG_MISC_SUPPORT */
#else
/* not support NV, dumy it. */
#define DBG_NV_ID       14
#define alog_nv_read(id, size, buf)         AIC_SUCCESS
#define alog_nv_write(id, size, buf, fs)    AIC_SUCCESS
#endif

#ifdef CONFIG_ALOG_CUSTOMER_CFG
#include <AIC_DEBUG.h>
#include <aic_debug_cfg.h>
#else
#include <alog_default_cfg.h>
#endif

/* log dev cfg api */
const ALOG_BUF_CFG_T *alog_get_buf_cfg(u8 type)
{
    AIC_DUMMY_ASSERT(type < ARRAY_SIZE(s_buf_cfg));
    return &s_buf_cfg[type];
}

const ALOG_CFG_T *alog_get_cfg(void)
{
    return &s_alog_cfg;
}

const ALBRR_CFG_T *alog_get_inputter_brr_cfg(u8 *cnt)
{
    *cnt = ARRAY_SIZE(s_inputter_cfg);
    return s_inputter_cfg;
}

/* log dev api */
u8 adbg_get_log_dev(void)
{
#ifdef CONFIG_SUBSYS_AP
    /* basic mode only can support uart. */
    if (aic_boot_mode_is_basic())
        return AIC_IODEV_UART;
#endif

    return ALOP_GET_DEV_TYPE(s_debug_cfg.aic_log.device);
}

void adbg_set_log_dev(u8 type)
{
    if (type >= AIC_IODEV_CNT ||
        type == ALOP_GET_DEV_TYPE(s_debug_cfg.aic_log.device))
        return;

    s_debug_cfg.aic_log.device = (type << 4 | (s_debug_cfg.aic_log.device & 0x0f));
    adbg_save_debug_nv_cfg(TRUE);
}

u8 adbg_get_log_bak_dev(void)
{
    return s_debug_cfg.aic_log.bak_dev;
}

void adbg_set_log_bak_dev(u8 bak_dev)
{
    if (bak_dev >= AIC_IODEV_CNT ||
        bak_dev == s_debug_cfg.aic_log.bak_dev)
        return;

    s_debug_cfg.aic_log.bak_dev = bak_dev;
    adbg_save_debug_nv_cfg(TRUE);
}

u8 adbg_get_log_dev_com(void)
{
    return ALOP_GET_DEV_COM(s_debug_cfg.aic_log.device);
}

void adbg_set_log_dev_com(u8 com)
{
    if (com >= AIC_COM_CNT ||
        com == ALOP_GET_DEV_COM(s_debug_cfg.aic_log.device))
        return;

    s_debug_cfg.aic_log.device = (com | (s_debug_cfg.aic_log.device & 0xf0));
    /* device com no need save to flash immediately now. */
    adbg_save_debug_nv_cfg(FALSE);
}

u32 adbg_get_log_baudrate(void)
{
    return s_debug_cfg.aic_log.baudrate;
}

void adbg_set_log_baudrate(u32 baudrate)
{
    if (baudrate == s_debug_cfg.aic_log.baudrate)
        return;

    s_debug_cfg.aic_log.baudrate = baudrate;
    adbg_save_debug_nv_cfg(TRUE);
}

/* diag dev api */
u8 adbg_get_diag_dev(void)
{
#ifdef CONFIG_SUBSYS_AP
    /* basic mode only can support uart. */
    if (aic_boot_mode_is_basic())
        return AIC_IODEV_UART;
#endif

    return ALOP_GET_DEV_TYPE(s_debug_cfg.aic_diag.device);
}

void adbg_set_diag_dev(u8 type)
{
    if (type >= AIC_IODEV_CNT ||
        type == ALOP_GET_DEV_TYPE(s_debug_cfg.aic_diag.device))
        return;

    s_debug_cfg.aic_diag.device = (type << 4 | (s_debug_cfg.aic_diag.device & 0x0f));
    adbg_save_debug_nv_cfg(TRUE);
}

u8 adbg_get_diag_dev_com(void)
{
    return ALOP_GET_DEV_COM(s_debug_cfg.aic_diag.device);
}

void adbg_set_diag_dev_com(u8 com)
{
    if (com >= AIC_COM_CNT ||
        com == ALOP_GET_DEV_COM(s_debug_cfg.aic_diag.device))
        return;

    s_debug_cfg.aic_diag.device = (com | (s_debug_cfg.aic_diag.device & 0xf0));
    /* device com no need save to flash immediately now. */
    adbg_save_debug_nv_cfg(FALSE);
}

u32 adbg_get_diag_baudrate(void)
{
    return s_debug_cfg.aic_diag.baudrate;
}

void adbg_set_diag_baudrate(u32 baudrate)
{
    if (s_debug_cfg.aic_diag.baudrate == baudrate)
        return;

    s_debug_cfg.aic_diag.baudrate = baudrate;
    adbg_save_debug_nv_cfg(TRUE);
}

void adbg_set_sys_debug(SYS_DBG_SW_T func, bool b_on)
{
    if (((s_debug_cfg.debug_gate & BIT(func)) > 0) == b_on)
        return;

    if (b_on)
        AIC_SET_BIT(s_debug_cfg.debug_gate, func);
    else
        AIC_CLR_BIT(s_debug_cfg.debug_gate, func);

    adbg_save_debug_nv_cfg(TRUE);
}

bool adbg_get_sys_debug(SYS_DBG_SW_T func)
{
    return ((s_debug_cfg.debug_gate & BIT(func)) > 0);
}

void adbg_set_sys_debug_ex(u32 gate)
{
    if (s_debug_cfg.debug_gate == gate)
        return;

    s_debug_cfg.debug_gate = gate;
    adbg_save_debug_nv_cfg(TRUE);
}

u32 adbg_get_sys_debug_ex(void)
{
    return s_debug_cfg.debug_gate;
}

/* dump and reset api */
void adbg_set_auto_reset(u8 sys, u8 reset)
{
    if (((s_debug_cfg.autoreset & BIT(sys)) > 0) == reset)
        return;

    if (reset)
        AIC_SET_BIT(s_debug_cfg.autoreset, sys);
    else
        AIC_CLR_BIT(s_debug_cfg.autoreset, sys);

    adbg_save_debug_nv_cfg(TRUE);
}

u8 adbg_is_auto_reset(u8 sys)
{
#ifdef CONFIG_SUBSYS_AP
    /* baisc mode, auto reset force set to 1. */
    if (aic_boot_mode_is_basic())
        return 1;
#endif
    return ((s_debug_cfg.autoreset & BIT(sys)) > 0);
}

void adbg_set_sleep_mode(u8 sleep_mode)
{
    if (sleep_mode)
        AIC_SET_BIT(s_debug_cfg.sleep_mode, 1);
    else
        AIC_CLR_BIT(s_debug_cfg.sleep_mode, 1);

    adbg_save_debug_nv_cfg(TRUE);
}

void adbg_set_buck0(u8 sleep_mode)
{
    if (sleep_mode)
        AIC_SET_BIT(s_debug_cfg.sleep_mode, 2);
    else
        AIC_CLR_BIT(s_debug_cfg.sleep_mode, 2);

    adbg_save_debug_nv_cfg(TRUE);
}

void adbg_set_buck1(u8 sleep_mode)
{
    if (sleep_mode)
        AIC_SET_BIT(s_debug_cfg.sleep_mode, 3);
    else
        AIC_CLR_BIT(s_debug_cfg.sleep_mode, 3);

    adbg_save_debug_nv_cfg(TRUE);
}

void adbg_set_uartdump(u32 port, u32 uart_dump)
{
#ifdef CONFIG_SUBSYS_AP
    if (s_debug_cfg.uart_datadump[port] == uart_dump)
        return;
    s_debug_cfg.uart_datadump[port] = uart_dump;
    adbg_save_debug_nv_cfg(TRUE);
#endif
}
u32 adbg_get_uartdump(u32 port)
{
#ifdef CONFIG_SUBSYS_AP
    return s_debug_cfg.uart_datadump[port];
#else
    return 0;
#endif
}
u8 adbg_is_sleep(void)
{
    return ((s_debug_cfg.sleep_mode & BIT_1) > 0);
}

u8 adbg_is_buck0_on(void)
{
    return ((s_debug_cfg.sleep_mode & BIT_2) > 0);
}

u8 adbg_is_buck1_on(void)
{
    return ((s_debug_cfg.sleep_mode & BIT_3) > 0);
}

void adbg_set_status_light(u8 sel, u8 light)
{
    if (light)
        AIC_SET_BIT(s_debug_cfg.status_light, sel);
    else
        AIC_CLR_BIT(s_debug_cfg.status_light, sel);

    adbg_save_debug_nv_cfg(TRUE);
}

u8 adbg_is_status_light(u8 sel)
{
    return ((s_debug_cfg.status_light & BIT(sel)) > 0);
}

void adbg_set_dump(u8 sys, u8 reset)
{
    if (((s_debug_cfg.sysdump & BIT(sys)) > 0) == reset)
        return;

    if (reset)
        AIC_SET_BIT(s_debug_cfg.sysdump, sys);
    else
        AIC_CLR_BIT(s_debug_cfg.sysdump, sys);

    adbg_save_debug_nv_cfg(TRUE);
}

u8 adbg_is_dump(u8 sys)
{
#ifdef CONFIG_SUBSYS_AP
    /* baisc mode, don't dump. */
    if (aic_boot_mode_is_basic())
        return 0;
#endif
    return ((s_debug_cfg.sysdump & BIT(sys)) > 0);
}

u32 adbg_get_at_com_bandrate(void)
{
    return s_debug_cfg.aic_at.baudrate;
}

void adbg_set_at_com_bandrate(u32 rate)
{
    if (s_debug_cfg.aic_at.baudrate == rate)
        return;

    s_debug_cfg.aic_at.baudrate = rate;
}

void adbg_set_log_pad(ALOG_PAD_SW_T func, bool b_on)
{
    if (((s_debug_cfg.aic_log.pad & BIT(func)) > 0) == b_on)
        return;

    if (b_on)
        AIC_SET_BIT(s_debug_cfg.aic_log.pad, func);
    else
        AIC_CLR_BIT(s_debug_cfg.aic_log.pad, func);

    adbg_save_debug_nv_cfg(TRUE);
}

bool adbg_get_log_pad(ALOG_PAD_SW_T func)
{
    return ((s_debug_cfg.aic_log.pad & BIT(func)) > 0);
}

void adbg_load_debug_nv_cfg(void)
{
    int ret;
    u8 lv, mod;
    AIC_DEBUG debug_nv_cfg = {0};

    ret = alog_nv_read(DBG_NV_ID, sizeof(debug_nv_cfg), &debug_nv_cfg);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
    if (ret)
        return;

    /* if it is not a valid nv, save default cfg to nv. */
    if (debug_nv_cfg.magic != ALOG_NV_MAGIC) {
        /* user mode disable some debug function default. */
        if (aic_is_user_mode()) {
            s_debug_cfg.sysdump = 0;
            s_debug_cfg.debug_gate = 0;
            s_debug_cfg.aic_log.gate = 0;
            s_debug_cfg.aic_log.device = (AIC_IODEV_NONE | (s_debug_cfg.aic_log.device & 0x0f));
        }

        /* set all module default log level. */
        for (mod = 0; mod < AMOD_CNT; mod++) {
            lv = aic_get_moudule_def_loglv(mod);
            alog_just_set_log_level(mod, lv);
        }

        adbg_save_debug_nv_cfg(FALSE);
        return;
    }

    /* use the nv cfg. */
    memcpy(&s_debug_cfg, &debug_nv_cfg, sizeof(s_debug_cfg));
}

void adbg_save_debug_nv_cfg(boolean forcesync)
{
    int ret;

    ret = alog_nv_write(DBG_NV_ID, sizeof(s_debug_cfg), &s_debug_cfg, forcesync);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
}

/* log api */
bool alog_get_log_switch(u8 mask)
{
    return s_debug_cfg.aic_log.gate & mask;
}

void alog_set_log_switch(bool enable, u8 mask)
{
    u8 gate = s_debug_cfg.aic_log.gate;

    if (enable)
        AIC_SET_MASK(gate, mask);
    else
        AIC_CLR_MASK(gate, mask);

#ifdef ALOG_FAST_API_FEATURE
    if (mask == ALOG_TRACE_SW)
        alog_set_fast_log((u8)enable);
#endif

    if (s_debug_cfg.aic_log.gate == gate)
        return;

    s_debug_cfg.aic_log.gate = gate;
    adbg_save_debug_nv_cfg(TRUE);
}

u8 alog_get_log_switch_ex(void)
{
    return s_debug_cfg.aic_log.gate;
}

void alog_set_log_switch_ex(u8 value)
{
    if (s_debug_cfg.aic_log.gate == value)
        return;

#ifdef ALOG_FAST_API_FEATURE
    if (value & ALOG_TRACE_SW)
        alog_set_fast_log(1);
    else
        alog_set_fast_log(0);
#endif

    s_debug_cfg.aic_log.gate = value;
    adbg_save_debug_nv_cfg(TRUE);
}

unsigned long *alog_get_level_bitmap(void)
{
    return s_debug_cfg.aic_log.level_bitmap;
}

#ifdef CONFIG_ALOG_MSG_SUPPORT
unsigned long *alog_get_msg_task_bitmap(void)
{
    return s_debug_cfg.aic_log.task_bitmap;
}
#endif

