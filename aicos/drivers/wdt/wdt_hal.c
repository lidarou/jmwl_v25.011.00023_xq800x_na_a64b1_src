/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of watchdog HAL module
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05da99
#define LOG_MODULE    AMOD_DRV

#include <aic_osa.h>
#include <aic_at_process_common.h>
#include <aic_cmdline.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include <core_op.h>
#include <irq_chip.h>
#include <wdt_cfg.h>
#include "wdt_phy.h"
#include <slp_inc.h>
#include <wdt.h>
#include <adbg_cfg_api.h>

#define WDT_PING_TIMER_NAME "wdt"

aos_timer_ptr   wdt_ping_timer = NULL;
static int wdt_work_mode = AIC_WDT_IRQ_RST_HYBRID_MODE;

#ifdef CONFIG_SW_WATCHDOG_MONITOR
aos_timer_ptr   sw_wdt_monitor_timer = NULL;
aos_event_ptr   sw_wdt_monitor_event;

#define         SW_WDT_TRIGGER_TIME     30000
#define         MONITOR_TASK_PRIROITY   35
#define         SW_WDT_MONITOR_KICK     0x1

void sw_watchdog_timeout(void *param)
{
    AICX_ASSERT_IFX(0, 0x1e28a6, "", "se");
}

void sw_watchdog_suspend(void *p)
{
    aos_deactivate_timer(sw_wdt_monitor_timer);
}

void sw_watchdog_resume(void *p)
{
    aos_change_timer(sw_wdt_monitor_timer, SW_WDT_TRIGGER_TIME, AOS_BE_PERIOD);
}

void sw_watchdog_kick(void)
{
    aos_set_event(sw_wdt_monitor_event, SW_WDT_MONITOR_KICK, AOS_OR);
}

void sw_watchdog_monitor_task(void *input)
{
    u32 e_bat_monitor;
    int ret;

    while(1) {
        ret = aos_get_event(sw_wdt_monitor_event, SW_WDT_MONITOR_KICK,
                            AOS_AND_CLEAR, &e_bat_monitor, AOS_WAIT_FOREVER);
        if (ret)
            continue;
        aos_sleep(100);
        aos_change_timer(sw_wdt_monitor_timer, SW_WDT_TRIGGER_TIME, AOS_BE_PERIOD);
    }
}

int sw_watchdog_init(void)
{
    sw_wdt_monitor_event = aos_create_event("swde");
    AIC_ASSERT(sw_wdt_monitor_event);

    aos_create_thread("swdt", sw_watchdog_monitor_task, AOS_NULL, 512, 0,
                      NULL, MONITOR_TASK_PRIROITY, AOS_AUTO_START);

    sw_wdt_monitor_timer = aos_create_timer("swdt",
                                         sw_watchdog_timeout, NULL,
                                         SW_WDT_TRIGGER_TIME,
                                         AOS_AUTO_ACTIVATE, AOS_BE_PERIOD);
    AIC_ASSERT(sw_wdt_monitor_timer);

    slp_register_sleep_hook(sw_watchdog_suspend, NULL, sw_watchdog_resume, NULL);

    return 0;
}
#endif

void aic_watchdog_ping(void *param)
{
    AIC_INT_SAVE_AREA

#ifdef CONFIG_SW_WATCHDOG_MONITOR
    sw_watchdog_kick();
#endif
    AIC_INT_DISABLE
    aic_wdt_kick();
    AIC_INT_RESTORE
}

void aic_watchdog_kick(void)
{
    aic_wdt_kick();
}

void aic_watchdog_start(void)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

#ifdef CONFIG_WATCHDOG_INDEPENDENT_THREAD
    aos_change_timer(wdt_ping_timer, WDT_PING_PERIOD_TIME, AOS_BE_PERIOD);
#endif

    aic_wdt_start();
    AIC_INT_RESTORE

#ifdef CONFIG_SW_WATCHDOG_MONITOR
    aos_change_timer(sw_wdt_monitor_timer, SW_WDT_TRIGGER_TIME, AOS_BE_PERIOD);
#endif
}

void aic_watchdog_stop(void)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    aic_wdt_enable();
    aic_wdt_stop();

#ifdef CONFIG_WATCHDOG_INDEPENDENT_THREAD
    if (wdt_ping_timer)
        aos_deactivate_timer(wdt_ping_timer);
#endif

    AIC_INT_RESTORE

#ifdef CONFIG_SW_WATCHDOG_MONITOR
    if (sw_wdt_monitor_timer)
        aos_deactivate_timer(sw_wdt_monitor_timer);
#endif
}

void aic_watchdog_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    aic_wdt_set_timeout(irq_timeout_ms, rst_timeout_ms);
    AIC_INT_RESTORE
}

void aic_watchdog_mode_config(AIC_WDT_MODE mode)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    aic_wdt_mode_config(mode);
    AIC_INT_RESTORE
}

u32 aic_watchdog_cnt_get(void)
{
    return aic_wdt_cnt_get();
}

static int aic_watchdog_irq_handler(void *param)
{
    aic_watchdog_stop();
    /*call system dump & system reset later.*/
    AICX_ASSERT_IFX(0, 0x0e46ad, "", "e");
    return IRQ_ISR_ONLY;
}

void wdt_reset(u32 time_ms)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

    aic_wdt_mode_config(AIC_WDT_RST_MODE_ONLY);

    aic_wdt_set_timeout(time_ms, time_ms);

    aic_wdt_start();

    aic_wdt_kick();

    while(1)
        ;
}

#ifdef CONFIG_WATCHDOG_INDEPENDENT_THREAD
void aic_watchdog_suspend(void *p)
{
    aic_wdt_kick();
    aos_deactivate_timer(wdt_ping_timer);
}

void aic_watchdog_resume(void *p)
{
    aic_watchdog_enable(wdt_work_mode, WDT_IRQ_TIMEOUT, WDT_RST_TIMEOUT);
    aos_change_timer(wdt_ping_timer, WDT_PING_PERIOD_TIME, AOS_BE_PERIOD);
}
#endif

int aic_watchdog_init(void)
{
    aic_wdt_reset();
    aic_wdt_enable();
    aos_request_irq(IRQ_WDG, aic_watchdog_irq_handler, NULL);

#ifdef CONFIG_SUBSYS_AP
    if (!aic_is_user_mode())
        wdt_work_mode = AIC_WDT_IRQ_MODE_ONLY;
#endif

#ifdef CONFIG_SW_WATCHDOG_MONITOR
    sw_watchdog_init();
#endif

#ifdef CONFIG_WATCHDOG_INDEPENDENT_THREAD
    wdt_ping_timer = aos_create_timer(WDT_PING_TIMER_NAME,
                                         aic_watchdog_ping, NULL,
                                         WDT_PING_PERIOD_TIME,
                                         AOS_AUTO_ACTIVATE, AOS_BE_PERIOD);
    AIC_ASSERT(wdt_ping_timer);
    aic_watchdog_enable(wdt_work_mode, WDT_IRQ_TIMEOUT, WDT_RST_TIMEOUT);
    slp_register_sleep_hook(aic_watchdog_suspend, NULL, aic_watchdog_resume, NULL);
#endif

    return 0;
}

AIC_DRV_P1_INIT_EXPORT(aic_watchdog_init);

#if defined (CONFIG_WATCHDOG_INDEPENDENT_THREAD) && (CONFIG_SUBSYS_AP)
static u32 at_wdt_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^WDT");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "ON/OFF.");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_wdt_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    ulong rsp[1];
    int ret = 0;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^WDT=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (!strcmp((char *)paras_arr[0],"ON") || !strcmp((char *)paras_arr[0],"on")
        || !strcmp((char *)paras_arr[0],"On")) {
        aic_watchdog_start();
        adbg_set_wdt_off(0);
    } else if (!strcmp((char *)paras_arr[0],"OFF") || !strcmp((char *)paras_arr[0],"off")
        || !strcmp((char *)paras_arr[0],"OFf")) {
        aic_watchdog_stop();
        adbg_set_wdt_off(1);
    } else {
        ret = AIC_ENXIO;
    }

    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

AT_CMD_HANDLE wdt_at_handles[] = {
    {
        "AT^WDT",
        at_wdt_test_cmd,
        AIC_NULL,
        at_wdt_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};


int wdt_at_cmd_init(void)
{
    if (aic_is_user_mode())
        return 0;

    aic_add_at_cmd_handle(&wdt_at_handles[0]);

    if (adbg_is_wdt_off())
        aic_watchdog_stop();

    return 0;
}
AIC_COMP_L2_INIT_EXPORT(wdt_at_cmd_init);
#endif
