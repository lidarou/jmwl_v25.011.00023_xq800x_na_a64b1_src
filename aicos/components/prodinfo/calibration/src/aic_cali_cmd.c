/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file process calibation diag command.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1da507
#define LOG_MODULE    AMOD_PRODINFO

#include <aic_type.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <aic_list.h>
#include <aic_boot_mode.h>
#ifdef CONFIG_SUBSYS_AP
#include <slp_inc.h>
#include <power_supply.h>
#include <aic_at_interact_cp.h>
#include <sysoff.h>
#endif
#include "aic_cali_cmd.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define CALI_NAME               "cali"
#define CALI_TASK_STACK_SIZE    1024
#define CALI_TASK_PRIORITY      6
#define CALI_QUEUE_NUM          10

#define CALI_ACQUIRE_MUTEX(mutex_ptr, wait_timeout) \
if (mutex_ptr && AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    u32 mutex_status; \
    mutex_status = aos_get_mutex(mutex_ptr, wait_timeout); \
    AIC_ASSERT(AOS_SUCCESS == mutex_status); \
}

#define CALI_RELEASE_MUTEX(mutex_ptr) \
if (mutex_ptr && AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    aos_put_mutex(mutex_ptr); \
}

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
/**
 * calibration callback.
 *
 * @func: calibration function.
 * @param: callback parameter.
 * @node: calibration node.
 */
typedef struct _cali_cb {
    CALI_FUNC func;
    void *param;
    struct list_head node;
} CALI_CB_T;

/**
 * calibration callback.
 *
 * @func: calibration function.
 * @param: callback parameter.
 * @node: calibration node.
 */
typedef enum {
    DIAG_NORMAL = 0,
    DIAG_GXEM,
    DIAG_RESET_GXEM,
} CALI_DIAG_MODE;

typedef enum {
    CALI_MSG_WAKEUP     = 0x1,
    CALI_MSG_SLEEP      = 0x2,
    CALI_MSG_MAX,
} CALI_MSG;

struct cali_queue_msg {
    CALI_MSG     msg;
    void        *data;
};

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
static aos_mutex_ptr cali_mutex = NULL;

LIST_HEAD(s_cali_list);

#ifdef CONFIG_SUBSYS_AP
static aos_thread_id        cali_thread_id;
static aos_queue_ptr        cali_queue;
#ifdef CONFIG_POWERON_DELAY_SLEEP_TIME
static aos_timer_ptr        cali_timer;
#endif
#endif

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_cali_register_hook -
 *  register callback to calibration, will be call it if calibration enter.
 *
 * Parameter:
 *  @func: input calibration callback.
 *  @param: input callback param.
 *
 * Return:
 */
void aic_cali_register_hook(CALI_FUNC func, void* param)
{
    CALI_CB_T *cali_cb = NULL;

    cali_cb = aos_zalloc(sizeof(CALI_CB_T));
    AIC_ASSERT(cali_cb);

    cali_cb->func = func;
    cali_cb->param = param;

    list_add(&cali_cb->node, &s_cali_list);
}

/**
 * aic_cali_unregister_hook - unregister callback to calibration.
 *
 * Parameter:
 *  @func: input calibration callback.
 *
 * Return:
 */
void aic_cali_unregister_hook(CALI_FUNC func)
{
    struct list_head *pos, *next;
    CALI_CB_T *cali_cb = NULL;

    list_for_each_safe(pos, next, &s_cali_list) {
        cali_cb = list_entry(pos, CALI_CB_T, node);
        if (cali_cb->func == func) {
            list_del(&cali_cb->node);
            aos_free(cali_cb);
        }
    }
}

/**
 * aic_cali_diag_handle - calibration diag handle.
 *
 * Parameter:
 * @diag_cmd: receive diag parameter
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_cali_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    struct list_head *pos, *next;
    CALI_CB_T *cali_cb = NULL;
    diag_rsp_cmd_t rsp;
    boolean chargemode;
    int errcode;

#ifdef CONFIG_SUBSYS_AP
    struct cali_queue_msg cali_msg;
    aos_status status;
#endif

    errcode = AIC_SUCCESS;

    switch (diag_cmd->subcmd) {
        case DIAG_GXEM:
            {
                if (aic_boot_mode_is_gxem()) {
                    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                                diag_cmd->m2, diag_cmd->sys, DIAG_CMD_GXEM,
                                diag_cmd->subcmd, NULL, 0);
                    diag_cmd_response(&rsp);
                    break;
                }

                chargemode = aic_boot_mode_is_charge();
                aic_boot_mode_set(BOOT_MODE_GXEM);

#ifdef CONFIG_SUBSYS_AP
                cali_msg.msg = CALI_MSG_WAKEUP;
                status = aos_send_to_queue(cali_queue, &cali_msg, AOS_WAIT_FOREVER);
                AIC_ASSERT(AOS_SUCCESS == status);

#ifdef CONFIG_BAT
                aic_battery_monitor_stop();
                psy_charging_enable(false);
#endif
                if (chargemode) {
                    subsys_power_on(AIC_SYS_CP);
                } else {
                    aic_send_cmd_and_wait("AT+CFUN=0", AIC_NULL);
                }

#ifdef CONFIG_D2D
#ifdef CONFIG_D2D_L3_DUMMY
                {
                    extern u32 aic_fd2d_api_disable_ext(void);
                    aic_fd2d_api_disable_ext();
                }
#else
                {
                    extern u32 stop_d2d_modem(void);
                    aicx_error(0x01616a, "", "call stop_d2d_modem() start\n");
                    stop_d2d_modem();
                    aicx_error(0x1cdaad, "", "call stop_d2d_modem() end\n");
                }
#endif
#endif
#endif

                list_for_each_safe(pos, next, &s_cali_list) {
                    cali_cb = list_entry(pos, CALI_CB_T, node);
                    cali_cb->func(aic_boot_mode_get(), cali_cb->param);
                }

                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                            diag_cmd->m2, diag_cmd->sys, DIAG_CMD_GXEM,
                            diag_cmd->subcmd, NULL, 0);
                diag_cmd_response(&rsp);
                break;
            }
        case DIAG_NORMAL:
            {
                aic_boot_mode_set(BOOT_MODE_NORMAL);
                list_for_each_safe(pos, next, &s_cali_list) {
                    cali_cb = list_entry(pos, CALI_CB_T, node);
                    cali_cb->func(aic_boot_mode_get(), cali_cb->param);
                }

#ifdef CONFIG_SUBSYS_AP
                cali_msg.msg = CALI_MSG_SLEEP;
                status = aos_send_to_queue(cali_queue, &cali_msg, AOS_WAIT_FOREVER);
                AIC_ASSERT(AOS_SUCCESS == status);

                aic_send_cmd_and_wait("AT+CFUN=1", AIC_NULL);
#ifdef CONFIG_BAT
                psy_charging_enable(true);
                aic_battery_monitor_start();
#endif
#endif

                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                            diag_cmd->m2, diag_cmd->sys, DIAG_CMD_GXEM,
                            diag_cmd->subcmd, NULL, 0);
                diag_cmd_response(&rsp);
                break;
            }
        case DIAG_RESET_GXEM:
            {
#ifdef CONFIG_RESET_ENTER_GXEM_SUPPORT
#ifdef CONFIG_BAT
                if (!aic_boot_mode_is_gxem()) {
                    aic_battery_monitor_stop();
                    psy_charging_enable(false);
                }
#endif

                sysoff_gxem_reset();
                break;
#endif
            }
        default:
            diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD,
                         diag_cmd->m2, diag_cmd->sys, DIAG_CMD_GXEM,
                         diag_cmd->subcmd, NULL, 0);
            diag_cmd_response(&rsp);
            break;
    }
}

#ifdef CONFIG_SUBSYS_AP
static void cali_entry(void *param)
{
    struct cali_queue_msg cali_msg;
    aos_status status;

#ifdef CONFIG_POWERON_DELAY_SLEEP_TIME
    aic_get_wakelock();
#else
    if (aic_boot_mode_is_gxem()) {
        aic_get_wakelock();
    }
#endif

#if 0//def CONFIG_UART_ENTER_GXEM
    diag_rsp_cmd_t rsp;
    diag_set_rsp(&rsp, 0x0, DIAG_RSP_SUCC, 0x0, 0x0, DIAG_CMD_GXEM, DIAG_GXEM, NULL, 0);
    diag_cmd_response(&rsp);
#endif

    while(1) {
        status = aos_recv_from_queue(cali_queue, &cali_msg, AOS_WAIT_FOREVER);
        AIC_ASSERT(AOS_SUCCESS == status);

        switch (cali_msg.msg) {
            case CALI_MSG_WAKEUP:
                aic_get_wakelock();
                break;
            case CALI_MSG_SLEEP:
                aic_put_wakelock();
            default:
                break;
        }
    }
}
#endif


#ifdef CONFIG_POWERON_DELAY_SLEEP_TIME
/**
 * aic_cali_poweron_delay_timer - power on delay timer callback.
 *
 * Parameter:
 *
 * Return:
 */
static void aic_cali_poweron_delay_timer(void *input)
{
    struct cali_queue_msg cali_msg;
    aos_status status;

    if (!aic_boot_mode_is_gxem()) {
        cali_msg.msg = CALI_MSG_SLEEP;
        status = aos_send_to_queue(cali_queue, &cali_msg, AOS_NO_WAIT);
        AIC_ASSERT(AOS_SUCCESS == status);
    }
}
#endif

/**
 * aic_cali_cmd_init - calibration diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_cali_cmd_init(void)
{
    cali_mutex = aos_create_mutex(CALI_NAME);
    AIC_ASSERT(cali_mutex);

    diag_register_cmd_handler(DIAG_CMD_GXEM, aic_cali_diag_handle, NULL);

#ifdef CONFIG_SUBSYS_AP
    cali_thread_id = aos_create_thread(CALI_NAME, cali_entry,
                                      AIC_NULL, CALI_TASK_STACK_SIZE,
                                      0, AIC_NULL, CALI_TASK_PRIORITY,
                                      AOS_AUTO_START);
    AOS_ASSERT(AOS_INVALID_THREAD_ID != cali_thread_id);

    cali_queue = aos_create_queue(CALI_NAME, sizeof(struct cali_queue_msg),
                                 CALI_QUEUE_NUM);
    AOS_ASSERT(cali_queue);

#ifdef CONFIG_POWERON_DELAY_SLEEP_TIME
    cali_timer = aos_create_timer(CALI_NAME,
                                    aic_cali_poweron_delay_timer, NULL,
                                    CONFIG_POWERON_DELAY_SLEEP_TIME * 1000,
                                    AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);
    AOS_ASSERT(cali_timer);
#endif
#endif

    return AIC_SUCCESS;
}

AIC_COMP_P2_INIT_EXPORT(aic_cali_cmd_init);

