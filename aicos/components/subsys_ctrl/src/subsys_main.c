
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the subsys main source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ef155
#define LOG_MODULE    AMOD_SUBSYS

#include <aic_assert.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aipc_api.h>
#include <aic_sys_param.h>
#include <aic_boot_mode.h>
#include <alog_store_api.h>
#include <adbg_cfg_api.h>
#include <assert_adpter.h>
#include <aic_list.h>
#include <subsys_api.h>
#include <subsys_loader.h>
#include <stdio.h>

#ifdef CONFIG_AIC_CTRL_SUPPORT
#include <aic_ctrl.h>
#include <aic_io_dev.h>
#endif

#ifdef WIN32
#define IRQ_CP_WDG_CP       0
#define IRQ_CP_WDG_ADSP     1
#else
#include <irq_cfg.h>
#endif

#ifdef CONFIG_SECURE_BOOT
#define SUBSYS_MAIN_STACK_SIZE      3072
#else
#define SUBSYS_MAIN_STACK_SIZE      1024
#endif

#define SUBSYS_LISTEN_STACK_SIZE    768

#define SUBSYS_MAIN_THREAD_PRI      3
#define SUBSYS_LISTEN_THREAD_PRI    3

#define SUBSYS_MAIN_QCNT        8
#define SUBSYS_ASSERT_KEY       "ASSERT:"
#define SUBSYS_ALIVE_KEY        "ALIVE:"

#define SUBSYS_MAIN_NAME        "sys-m"
#define CP_SYS_NAME             "cpsys"
#define CP_LISTEN_NAME          "cplt"

#ifdef CONFIG_SUPPORT_ADSP
#define ADSP_SYS_NAME           "adsys"
#define ADSP_LISTEN_NAME        "adlt"
#endif

/* default don't boot adsp, it will be boot by audio module. */
#define AUTO_BOOT_ADSP  0

typedef struct subsys_state_cb {
    subsys_state_nf     nf;
    void                *p;
    struct list_head    next;
} subsys_state_cb_t;

typedef struct subsys_main {
    u8              rsv;
    u8              sys;
    u8              link;
    u8              state;
    u16             irq;
    u16             boot_cnt;
    void            *brr;
    char            *name;
    subsys_loader_t *loader;
    aos_thread_id   listen_thread;
    struct list_head notify_head;
} subsys_main_t;

static subsys_main_t *s_cp_subsys;
#ifdef CONFIG_SUPPORT_ADSP
static subsys_main_t *s_adsp_subsys;
#endif

static aos_thread_id   s_main_thread;
static aos_queue_ptr   s_mian_queue;

static subsys_main_t *subsys_main_get_sys(u8 sys)
{
    switch (sys) {
        case AIC_SYS_CP:
        return s_cp_subsys;

#ifdef CONFIG_SUPPORT_ADSP
        case AIC_SYS_ADSP:
        return s_adsp_subsys;
#endif

        default:
        return NULL;
    }
}

static void subsys_main_set_state(subsys_main_t *subsys, u8 state)
{
    struct list_head *head = &subsys->notify_head;
    subsys_state_cb_t *pos;

    if (state !=  subsys->state) {
        aicx_debug(0x1da0e8, "sdd", "%s:state %d ==> %d\n", subsys->name, subsys->state, state);
        subsys->state = state;

        /* find the channel handler to process it. */
        list_for_each_entry(pos, head, next, subsys_state_cb_t) {
            pos->nf(state, pos->p);
        }
    }
}

void subsys_main_send_msg(u16 sys, u16 action)
{
    ulong msg = (sys << 16 | action);

    /* add to msg queue */
    (void)aos_send_to_queue(s_mian_queue, (void*)&msg, AOS_NO_WAIT);
}

#define subsys_main_send_msg_inter(sys, action) do { \
    ulong msg = (sys << 16 | action); \
    (void)aos_send_to_queue(s_mian_queue, (void*)&msg, AOS_NO_WAIT); \
} while (0)

static int subsys_main_wdg_handler(void *p)
{
    subsys_main_t *subsys = p;

    aos_release_irq(subsys->irq);
    aicx_debug(0x01b256, "s", "%s wdg timeout\n", subsys->name);

#ifndef WIN32
    /* save cp info, and try assert cp. */
    ap_save_cp_reg_info();
    ap_notify_other_sys_assert(AIC_SYS_CP);
#endif

    subsys_main_send_msg_inter(subsys->sys, SUBSYS_MAIN_MSG_WDG);
    return 0;
}

static void subsys_main_boot_sys(subsys_main_t *subsys, bool p_on)
{
    subsys->boot_cnt++;

#if CONFIG_CP_RESET_ONLY_CP
    /* add bootcnt to cp cmdline. */
    if (subsys->sys == AIC_SYS_CP) {
        char bootcnt_str[10];

        snprintf(bootcnt_str, sizeof(bootcnt_str), "%d", subsys->boot_cnt);
        aic_cmdline_update_key(cl_key_bootcnt, bootcnt_str);
    }

    /*
     * boot cp:
     *    first boot, cp is power off, just power on it.
     *    reboot cp, cp is power on, just start it.
     * boot adsp:
     *    always power on it.
     */

    /* start */
    if (!p_on) {
        subsys_loader_load(subsys->loader);
        subsys_loader_start(subsys->loader);
        return;
    }
#endif

    /* power on */
    if (subsys->sys == AIC_SYS_CP) {
        subsys_loader_load(subsys->loader);
        subsys_loader_power_on(subsys->loader);
    } else {
        subsys_loader_power_on(subsys->loader);
        subsys_loader_stop(subsys->loader);
        subsys_loader_load(subsys->loader);
        subsys_loader_start(subsys->loader);
    }

    /* request subsys watchdog irq handle. */
    aos_request_irq(subsys->irq, subsys_main_wdg_handler, subsys);
}

static void subsys_main_poweron_subysys(subsys_main_t *subsys, bool delay)
{
    /*
     * Notify: this below code only for cp sys:
     * 1. Cp only can power on once, but adsp need power on again and agian.
     * 2. CP has the delay boot option, but adsp does not.
     */
    if (subsys->sys == AIC_SYS_CP) {
        /* has power on, do nothing. */
        if (subsys->boot_cnt)
            return;

        if (delay)
            aos_sleep(10*1000);
    }

    subsys_main_set_state(subsys, SUBSYS_STATE_BOOTING);
    subsys_main_boot_sys(subsys, TRUE);
}

static void subsys_main_fisrt_boot_subysys(void)
{
    bool delay = adbg_is_cp_delay_boot();

#if IS_CONF_ENABLED(AUTO_BOOT_ADSP)
    /* boot adsp */
    subsys_main_poweron_subysys(s_adsp_subsys, FALSE);
#endif

    /* boot cp */
    subsys_main_poweron_subysys(s_cp_subsys, delay);
}

void subsys_main_thread(void *p)
{
    subsys_main_t *subsys;
    u16 action, sys;
    ulong msg;
    int ret;

    /* charge mode don't boocp and adsp here. */
    if (!aic_boot_mode_is_charge()) {
        subsys_main_fisrt_boot_subysys();
    }

    /* process message */
    while (TRUE) {
        ret = aos_recv_from_queue(s_mian_queue, (void*)&msg, AIPC_WAIT_FOREVER);
        if (ret != AOS_SUCCESS)
            continue;

        sys = msg >> 16;
        action = msg & 0xFFFF;
        subsys = subsys_main_get_sys((u8)sys);
        if (!subsys)
            continue;

        aicx_info(0x086233, "sd", "subsys %s main recv msg=%d\n", subsys->name, action);

        switch (action) {
        case SUBSYS_MAIN_MSG_ALIVE:
            subsys_main_set_state(subsys, SUBSYS_STATE_ALIVE);
            break;

        case SUBSYS_MAIN_MSG_ASSERT:
            subsys_main_set_state(subsys, SUBSYS_STATE_ASSERT);
            break;

        case SUBSYS_MAIN_MSG_RESET:
            subsys_main_set_state(subsys, SUBSYS_STATE_RESET);
            break;

        case SUBSYS_MAIN_MSG_BLOCK:
            subsys_main_set_state(subsys, SUBSYS_STATE_BLOCK);
            break;

        case SUBSYS_MAIN_MSG_WDG:
            subsys_main_set_state(subsys, SUBSYS_STATE_WDG);
            break;

#if CONFIG_CP_RESET_ONLY_CP
        case SUBSYS_MAIN_MSG_STOP:
            subsys_main_set_state(subsys, SUBSYS_STATE_STOP);
            subsys_loader_stop(subsys->loader);
            aos_release_irq(subsys->irq);
            break;

        case SUBSYS_MAIN_MSG_START:
            subsys_main_set_state(subsys, SUBSYS_STATE_BOOTING);
            subsys_main_boot_sys(subsys, FALSE);
            break;
#endif

        case SUBSYS_MAIN_MSG_POWER_OFF:
            subsys_main_set_state(subsys, SUBSYS_STATE_POWER_OFF);
            subsys_loader_power_off(subsys->loader);
            aos_release_irq(subsys->irq);
            break;

        case SUBSYS_MAIN_MSG_POWER_ON:
            subsys_main_poweron_subysys(subsys, FALSE);
            break;

        case SUBSYS_MAIN_MSG_DUMP:
            subsys_loader_dump_sys(subsys->sys);
            break;

        default:
            break;
        }
    }
}

void subsys_listen_thread(void *p)
{
    subsys_main_t *subsys = (subsys_main_t *)p;
    ABLK_T blk;
    int ret;
    const char *info;

    aicx_debug(0x17e17a, "s", "%s: listen thread!\n", subsys->name);

    while (1) {
        ret = ablk_recv_block(subsys->brr, subsys->link, &blk, AIPC_WAIT_FOREVER);
        if (ret != AIC_SUCCESS) {
            aos_sleep(100);
            continue;
        }

        info = (const char *)blk.addr;
        aicx_info(0x0281da, "ss", "%s recv: %s\n", subsys->name, info);

        if (strstr(info, SUBSYS_ALIVE_KEY)) {
            subsys_main_send_msg_inter(subsys->sys, SUBSYS_MAIN_MSG_ALIVE);
        } else if (strstr(info, SUBSYS_ASSERT_KEY)) {

#ifdef CONFIG_AIC_CTRL_SUPPORT
        aic_ctrl_str_notify(info);
        aos_sleep(200);
#endif

#ifdef CONFIG_ALOG_STORE_FS
            /* basic mode can't call it. */
            if (!aic_boot_mode_is_basic())
                alog_store_save_assert_info((char *)info, 0, TRUE);
#endif
            subsys_main_send_msg_inter(subsys->sys, SUBSYS_MAIN_MSG_ASSERT);
        } else if (strstr(info, CP_RESET_ACK)) {
            subsys_main_send_msg_inter(subsys->sys, SUBSYS_MAIN_MSG_RESET);
        }
#ifdef CONFIG_AIC_CTR_DBG_ON_CMUX
        else if (AIC_IODEV_CMUX == adbg_get_log_dev() &&
                 0 == strcmp(info, DUMP_FINISH)) {
            aic_ctrl_str_notify(DUMP_FINISH);
        }
#endif
        /* release the blk. */
        ablk_release_block(subsys->brr, &blk);
    }
}

void *subsys_main_create(u8 sys, u8 brr, int irq,
                         u8 link, char *name, char *monitot_name)
{
    subsys_main_t *subsys;

    aicx_debug(0x08de02, "s", "subsys_main_create %s\n", name);

    subsys = (subsys_main_t *)aos_zalloc(sizeof(*subsys));
    AIC_DUMMY_ASSERT(subsys);

    subsys->sys = sys;
    subsys->irq = irq;
    subsys->link = link;
    subsys->state = SUBSYS_STATE_INIT;
    subsys->name = name;

    list_head_init(&subsys->notify_head);

    subsys->loader = subsys_loader_create(sys, name);
    AIC_DUMMY_ASSERT(subsys->loader);

    /* if monitot_name is NULL, no need listsen. */
    if (monitot_name) {
        subsys->brr = ablk_request_brr(sys, brr);
        AIC_DUMMY_ASSERT(subsys->brr);

        subsys->listen_thread =
            aos_create_thread_simply(monitot_name, subsys_listen_thread, subsys,
                                     SUBSYS_LISTEN_STACK_SIZE,
                                     SUBSYS_LISTEN_THREAD_PRI);
        AIC_DUMMY_ASSERT(subsys->listen_thread);
    }

    return subsys;
}

u8 subsys_get_state(u8 sys)
{
    subsys_main_t *subsys = subsys_main_get_sys(sys);

    if (!subsys)
        return SUBSYS_STATE_POWER_OFF;

    return subsys->state;
}

void subsys_wait_poweroff(u8 sys, int timeout)
{
    subsys_main_t *subsys = subsys_main_get_sys(sys);
    int cnt = timeout > 10 ? timeout/10 : timeout;
    int time_sleep = timeout > 10 ? 10: 1;

    if (!subsys)
        return;

    while(!subsys_loader_is_poweroff(subsys->loader) && cnt > 0) {
            aos_sleep(time_sleep);
            cnt--;
    }

    if (!subsys_loader_is_poweroff(subsys->loader)) {
        aicx_info(0x050748, "d", "timeout, force poweroff sys(%d).", sys);

        /* force power off. */
        subsys_loader_power_off(subsys->loader);
        aos_sleep(1);
        if (!subsys_loader_is_poweroff(subsys->loader)) {
            aicx_info(0x09dc30, "d", "wait sys(%d) power off timeout.", sys);
        }
    }
}

void subsys_add_state_notify(u8 sys, subsys_state_nf nf, void *p)
{
    subsys_main_t *subsys = subsys_main_get_sys(sys);
    subsys_state_cb_t *state_cb;

    if (subsys) {
        state_cb = aos_malloc(sizeof(*state_cb));
        AIC_DUMMY_ASSERT(state_cb);
        state_cb->nf = nf;
        state_cb->p = p;
        list_add(&state_cb->next, &subsys->notify_head);
    }
}

void subsys_main_del_state_notify(u8 sys, void *p)
{
    subsys_main_t *subsys = subsys_main_get_sys(sys);
    subsys_state_cb_t *pos, *temp;
    struct list_head *head;

    if (!subsys)
        return;

    head = &subsys->notify_head;
    /* find p, than del it*/
    list_for_each_entry_safe(pos, temp, head, next, subsys_state_cb_t) {
        if (p == pos->p) {
            list_del(&pos->next);
            aos_free(pos);
            return;
        }
    }
}

static void subsys_main_release(subsys_main_t *subsys)
{
    subsys_state_cb_t *pos, *temp;
    struct list_head *head = &subsys->notify_head;

    /* release list */
    list_for_each_entry_safe(pos, temp, head, next, subsys_state_cb_t) {
            list_del(&pos->next);
            aos_free(pos);
    }

    /* relase loader */
    subsys_loader_release(subsys->loader);

    if (subsys->brr) {
        ablk_release_brr(subsys->brr);
        aos_delete_thread(subsys->listen_thread);
    }

    aos_free(subsys);
}

void subsys_deinit(void)
{
    subsys_main_release(s_cp_subsys);

#ifdef CONFIG_SUPPORT_ADSP
    subsys_main_release(s_adsp_subsys);
#endif

    aos_delete_queue(s_mian_queue);

    aos_delete_thread(s_main_thread);
}

int subsys_init(void)
{
    aicx_debug(0x13c28f, "", "subsys_init\n");

#ifdef CONFIG_FOTA_BY_MCU
    /* fota by mcu, basic mode don't boot subsys. */
    if (aic_boot_mode_is_basic()) {
        /* mpu config cp region. */
        subsys_mpu_config_sys(AIC_SYS_CP);
        return AIC_SUCCESS;
    }
#endif

    s_main_thread =
        aos_create_thread_simply(SUBSYS_MAIN_NAME, subsys_main_thread, NULL,
                                 SUBSYS_MAIN_STACK_SIZE,
                                 SUBSYS_MAIN_THREAD_PRI);

    AIC_DUMMY_ASSERT(s_main_thread);


    s_mian_queue = aos_create_queue(SUBSYS_MAIN_NAME, sizeof(void*), SUBSYS_MAIN_QCNT);
    AIC_DUMMY_ASSERT(s_mian_queue);

    s_cp_subsys = subsys_main_create(AIC_SYS_CP, AIPC_BRR_COMM, IRQ_CP_WDG_CP,
                                     STATUS_LINK, CP_SYS_NAME, CP_LISTEN_NAME);
    AIC_DUMMY_ASSERT(s_cp_subsys);

#ifdef CONFIG_SUPPORT_ADSP
    s_adsp_subsys = subsys_main_create(AIC_SYS_ADSP, AIPC_BRR_COMM, IRQ_CP_WDG_ADSP,
                                       STATUS_LINK, ADSP_SYS_NAME, /* ADSP_LISTEN_NAME */ NULL);
    AIC_DUMMY_ASSERT(s_adsp_subsys);
#endif

    return AIC_SUCCESS;
}
AIC_COMP_P2_INIT_EXPORT(subsys_init);

