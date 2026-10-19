/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *    The aic debug sys api.h
 */
#ifndef __ADBG_CFG_API_H__
#define __ADBG_CFG_API_H__

#include <aic_type.h>
#include <aic_sys.h>
/*
 * sys debug switch define, every function onwn 1 bit.
 * can  support max 32 switchs.
 * The default value of every switch is on in debug version,
 * And is off in user version.
 */
typedef enum {
    SYS_DBG_TRACEX = 0,
    SYS_DBG_DBUS,
    SYS_DBG_PERF,
    SYS_DBG_CHG_PD,
    SYS_DBG_WDT_OFF,
    SYS_DBG_QUICK_SCREEN_OFF,
} SYS_DBG_SW_T;

void adbg_set_sys_debug_ex(u32 gate);
u32 adbg_get_sys_debug_ex(void);

void adbg_set_sys_debug(SYS_DBG_SW_T func, bool b_on);
bool adbg_get_sys_debug(SYS_DBG_SW_T func);

#define adbg_set_tracex_on(on)  adbg_set_sys_debug(SYS_DBG_TRACEX, (on))
#define adbg_is_tracex_on()     adbg_get_sys_debug(SYS_DBG_TRACEX)
#define adbg_set_dbus_on(on)    adbg_set_sys_debug(SYS_DBG_DBUS, (on))
#define adbg_is_dbus_on()       adbg_get_sys_debug(SYS_DBG_DBUS)
#define adbg_set_perf(on)       adbg_set_sys_debug(SYS_DBG_PERF, (on))
#define adbg_is_perf()          adbg_get_sys_debug(SYS_DBG_PERF)
#define adbg_set_chg_pd(on)     adbg_set_sys_debug(SYS_DBG_CHG_PD, (on))
#define adbg_is_chg_pd()        adbg_get_sys_debug(SYS_DBG_CHG_PD)
#define adbg_set_wdt_off(on)    adbg_set_sys_debug(SYS_DBG_WDT_OFF, (on))
#define adbg_is_wdt_off()       adbg_get_sys_debug(SYS_DBG_WDT_OFF)
#define adbg_set_qs_off(on)     adbg_set_sys_debug(SYS_DBG_QUICK_SCREEN_OFF, (on))
#define adbg_is_qs_off()        adbg_get_sys_debug(SYS_DBG_QUICK_SCREEN_OFF)

/* log_pad, BIT0: for cp log, delay boot cp 10S. */
typedef enum {
    ALOG_DELAY_BOOT_CP = 0,
    ALOG_DUMP_CP_BY_LOG
} ALOG_PAD_SW_T;

bool adbg_get_log_pad(ALOG_PAD_SW_T func);
void adbg_set_log_pad(ALOG_PAD_SW_T func, bool b_on);

#define adbg_set_ldump_on(on)       adbg_set_log_pad(ALOG_DUMP_CP_BY_LOG, (on))
#define adbg_is_ldump_on(on)        adbg_get_log_pad(ALOG_DUMP_CP_BY_LOG)
#define adbg_set_cp_delay_boot(on)  adbg_set_log_pad(ALOG_DELAY_BOOT_CP, (on))
#define adbg_is_cp_delay_boot(on)   adbg_get_log_pad(ALOG_DELAY_BOOT_CP)

/* log dev api */
u8 adbg_get_log_dev(void);
void adbg_set_log_dev(u8 type);
u8 adbg_get_log_bak_dev(void);
void adbg_set_log_bak_dev(u8 bak_dev);
u8 adbg_get_log_dev_com(void);
void adbg_set_log_dev_com(u8 type);
u32 adbg_get_log_baudrate(void);
void adbg_set_log_baudrate(u32 baudrate);

/* diag dev api */
u8 adbg_get_diag_dev(void);
void adbg_set_diag_dev(u8 type);
u8 adbg_get_diag_dev_com(void);
void adbg_set_diag_dev_com(u8 type);
void adbg_set_diag_baudrate(u32 baudrate);
u32 adbg_get_diag_baudrate(void);

/* dump and reset api */
void adbg_set_auto_reset(u8 sys, u8 reset);
u8 adbg_is_auto_reset(u8 sys);
void adbg_set_dump(u8 sys, u8 reset);
u8 adbg_is_dump(u8 sys);

/* sleep mode*/
void adbg_set_sleep_mode(u8 sleep_mode);
void adbg_set_buck0(u8 sleep_mode);
void adbg_set_buck1(u8 sleep_mode);
u8 adbg_is_sleep(void);
u8 adbg_is_buck0_on(void);
u8 adbg_is_buck1_on(void);

/*uart debug*/
void adbg_set_uartdump(u32 port, u32 uart_dump);
u32 adbg_get_uartdump(u32 port);
/*net/device/module status light*/
void adbg_set_status_light(u8 sel, u8 light);
u8 adbg_is_status_light(u8 sel);

/* at com bandrate api */
u32 adbg_get_at_com_bandrate(void);
void adbg_set_at_com_bandrate(u32 rate);

/* load and save debug cfg api */
void adbg_load_debug_nv_cfg(void);
void adbg_save_debug_nv_cfg(boolean forcesync);

#define adbg_set_ap_reset(reset)        adbg_set_auto_reset(AIC_SYS_AP, reset)
#define adbg_is_ap_reset()              adbg_is_auto_reset(AIC_SYS_AP)
#define adbg_set_cp_reset(reset)        adbg_set_auto_reset(AIC_SYS_CP, reset)
#define adbg_is_cp_reset()              adbg_is_auto_reset(AIC_SYS_CP)
#define adbg_set_adsp_reset(reset)      adbg_set_auto_reset(AIC_SYS_ADSP, reset)
#define adbg_is_adsp_reset()            adbg_is_auto_reset(AIC_SYS_ADSP)

#define adbg_set_ap_dump(dump)          adbg_set_dump(AIC_SYS_AP, dump)
#define adbg_is_ap_dump()               adbg_is_dump(AIC_SYS_AP)
#define adbg_set_cp_dump(dump)          adbg_set_dump(AIC_SYS_CP, dump)
#define adbg_is_cp_dump()               adbg_is_dump(AIC_SYS_CP)
#define adbg_set_adsp_dump(dump)        adbg_set_dump(AIC_SYS_ADSP, dump)
#define adbg_is_adsp_dump()             adbg_is_dump(AIC_SYS_ADSP)

#endif /* __ADBG_CFG_API_H__ */

