
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
#define AIC_THIS_FILE    0x0133f5
#define LOG_MODULE    AMOD_SUBSYS

#include <aic_assert.h>
#include <aic_boot_mode.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <aipc_init.h>
#include <adbg_cfg_api.h>
#include <assert_adpter.h>
#include <subsys_api.h>
#include <aic_mcx.h>
#include <sysoff.h>
#include <slp_inc.h>

#define WAIT_CP_ALIVE_TIME          5000
#define WAIT_CP_RESE_ACK_TIME       1000

enum {
    WAIT_NONE = 0,
    WAIT_CP_ALIVE,
    WAIT_CP_RESET_ACK
};

u16 cp_reset_cnt;
static u8 s_wait_state;
static aos_timer_ptr s_wait_cp_timer;

static void cp_state_reboot_sys(int reason)
{
     /* reboot sys */
     sysoff_set_dumpmode(reason);

     if (aic_boot_mode_is_basic())
        sysoff_reset_basic();
     else
        sysoff_reset();
}

static void cp_state_reset_cp(void)
{
#if IS_CONF_ENABLED(CONFIG_CP_RESET_ONLY_CP)
    /* reset cp only, otherwise reset all chip. */
    subsys_stop(AIC_SYS_CP);
    subsys_start(AIC_SYS_CP);

    aicx_debug(0x01cf80, "", "After cp assert, reset CP and wait alive!.\n");

    s_wait_state = WAIT_CP_ALIVE;
    aos_change_timer(s_wait_cp_timer, WAIT_CP_ALIVE_TIME, FALSE);
    aos_activate_timer(s_wait_cp_timer);
#else
    aicx_debug(0x1e5193, "", "After cp assert reset AP.\n");
    /* reboot sys */
    cp_state_reboot_sys(SYSOFF_MODE_CPCRASH);
#endif /* CONFIG_CP_RESET_ONLY_CP */
}

void cp_state_process(subsys_state_t state, void *p)
{
    aicx_info(0x007de4, "d", "cp_state_process state=%d.\n", state);

    switch (state) {
    case SUBSYS_STATE_ASSERT:
        /* get a wake lock for cp assert proc diag cmd. */
        aic_get_wakelock();

        /* send a cmd to cp for output the assertinfo. */
        ap_send_cmd_to_cp(DIAG_CMD_ASSERT, ASSERT_ALLINFO_SUBCMD);

        if (adbg_is_cp_dump()) {
            /* send cmd to cp for output the dumpinfo. */
            ap_send_cmd_to_cp(DIAG_CMD_ASSERT, ASSERT_DUMP_SUBCMD);
            /* the pc will send rest diag to cp, and not start wait alive timer. */
            s_wait_state = WAIT_CP_RESET_ACK;
            break;
        }

        if (adbg_is_cp_reset()) {
            /* send reset cmd to cp. */
            ap_send_cmd_to_cp(DIAG_CMD_ASSERT, ASSERT_RESET_SUBCMD);
            s_wait_state = WAIT_CP_RESET_ACK;
            aos_change_timer(s_wait_cp_timer, WAIT_CP_RESE_ACK_TIME, FALSE);
            aos_activate_timer(s_wait_cp_timer);
            break;
        }
        break;

    case SUBSYS_STATE_RESET:
        cp_reset_cnt++;
        /* cp ready reset. */
        aicx_info(0x1572ae, "d", "recv cp reset ack, Reboot cp, cnt=%d.\n", cp_reset_cnt);

#if IS_CONF_ENABLED(CONFIG_CP_RESET_ONLY_CP)
        /*  reset the aipc sys. */
        aipc_reset_sys(AIC_SYS_CP);
#endif
        cp_state_reset_cp();

        /* put the wake lock after reset cp. */
        aic_put_wakelock();
        break;

    case SUBSYS_STATE_WDG:
    case SUBSYS_STATE_HUNG:
    case SUBSYS_STATE_BLOCK:
        aic_get_wakelock();

#ifdef CONFIG_MCX_SUPPORT
        /* disable mcx. */
        mcx_eb_set(FALSE);
        mcx_ap_force_set_event();
#endif
        if (adbg_is_cp_dump()) {
            subsys_dump(AIC_SYS_CP);
       } else if(adbg_is_cp_reset()) {
            /* cp block and hung, reboot the whole sys. */
            aicx_info(0x06cc28, "", "cp hung or block, Reboot sys.\n");
            /* reboot sys */
            if (state == SUBSYS_STATE_WDG)
                cp_state_reboot_sys(SYSOFF_MODE_CPWDG);
            else
                cp_state_reboot_sys(SYSOFF_MODE_CPCRASH);
        }
    break;

    case SUBSYS_STATE_ALIVE:
        aos_deactivate_timer(s_wait_cp_timer);
        s_wait_state = WAIT_NONE;
        break;

    case SUBSYS_STATE_BOOTING:
        if (s_wait_state == WAIT_CP_ALIVE)
            break;

        /* start wait alive. */
        s_wait_state = WAIT_CP_ALIVE;
        if (adbg_is_cp_reset()) {
            aos_change_timer(s_wait_cp_timer, WAIT_CP_ALIVE_TIME, FALSE);
            aos_activate_timer(s_wait_cp_timer);
        }
    break;

    case SUBSYS_STATE_STOP:
    case SUBSYS_STATE_POWER_OFF:
        aos_deactivate_timer(s_wait_cp_timer);
        break;

    case SUBSYS_STATE_INIT:
    default:
        /* do nothing */
        break;
    }
}

void state_wait_cp_timer_func(void *input)
{
    aicx_debug(0x150de9, "d", "wait cp alive timeout, state =%d!", s_wait_state);

    if (adbg_is_cp_dump()) {
        /* save cp info, and try assert cp. */
        ap_save_cp_reg_info();
        ap_notify_other_sys_assert(AIC_SYS_CP);
        subsys_dump(AIC_SYS_CP);
    } else {
        /* set cp state to reset for, reboot sys */
        subsys_set_reset_state(AIC_SYS_CP);
    }
}

int subsys_state_porc_init(void)
{
    subsys_add_state_notify(AIC_SYS_CP, cp_state_process, NULL);

    s_wait_cp_timer = aos_create_timer("wait_cp",
                                       state_wait_cp_timer_func, NULL,
                                       WAIT_CP_ALIVE_TIME,
                                       AOS_NO_ACTIVATE, AOS_NO_PERIOD);

    return AIC_SUCCESS;
}

/*
 * ifdef CONFIG_AIC_CTRL_SUPPORT,
 * all the subsys state will be processed by aic_ctrl.
 */
#ifndef CONFIG_AIC_CTRL_SUPPORT
DIS_CHARGE_APP_P2_INIT_EXPORT(subsys_state_porc_init);
#endif

