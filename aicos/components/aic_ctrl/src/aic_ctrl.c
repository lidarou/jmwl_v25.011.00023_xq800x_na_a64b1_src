
/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the aic ctrl source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x179f12
#define LOG_MODULE    AMOD_SYS

#include <aic_assert.h>
#include <aic_ctrl.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <cmux.h>
#include "cmux_cfg.h"
#include <ext_modem.h>
#include <gpio_inc.h>
#include <subsys_api.h>
#include <sysoff.h>
#include <alog_cfg.h>
#include <aic_io_dev.h>
#include <aic_cmdline.h>
#include <adbg_cfg_api.h>
#include <alog_diag_cmd.h>
#include <diag_cmd_api.h>
#include <assert_adpter.h>

/* aic state */
#define AIC_CTRL_BOOTED         "booted"
#define AIC_CTRL_ALIVE          "alive"
#define AIC_CTRL_ASSERT         "ASSERT"
#define AIC_CTRL_SHUTDOWN       "shutdown"


/* aic mode */
#define AIC_CTRL_NORMAL         "normal"
#define AIC_CTRL_ENG            "eng"
#define AIC_CTRL_CHG            "chg"
#define AIC_CTRL_BAISC          "basic"

/* aic cmd */
#define AIC_CTRL_SET_CONFIG         "config"
#define AIC_CTRL_INTER_MODEM_DBG    "DBG"
#define AIC_CTRL_INTER_CPLOG        "CPLOG"
#define AIC_CTRL_STR_ON             "ON"
#define AIC_CTRL_STR_OFF            "OFF"

#define AIC_CTRL_STACK_SIZE     1024
#define AIC_CTRL_THREAD_PRI     5

#define AIC_CTRL_CFG_DATA_LEN       (128)/*need same with MCU*/
#define AIC_CTRL_CFG_MODULE_LEN     (20)/*need same with MCU*/
/* AIC_CTRL_CFG_BUFFER_LEN = sizeof(aic_ctrl_inter_cfg_t) + strlen("config") */
#define AIC_CTRL_CFG_BUFFER_LEN     ((AIC_CTRL_CFG_DATA_LEN + 1) + (AIC_CTRL_CFG_MODULE_LEN + 1) + 6)

typedef struct aic_ctrl {
    void            *ctrl_mux;
    aos_thread_id   recv_thread;
    aos_thread_id   send_thread;
    aos_queue_ptr   send_queue;
    aos_mutex_ptr   cfg_mutex;
    struct list_head cfg_cb_list;
} aic_ctrl_t;

typedef struct {
    char    module[AIC_CTRL_CFG_MODULE_LEN + 1];
    uint8_t cfg_data[AIC_CTRL_CFG_DATA_LEN + 1];
} aic_ctrl_cfg_t;

typedef struct aic_ctrl_srv_cb {
    char                module[AIC_CTRL_CFG_MODULE_LEN + 1];
    void                *cfg_callback;
    struct list_head    next;
} aic_ctrl_cfg_cb_t;

aic_ctrl_t s_aic_ctrl;

int32_t aic_ctrl_set_config_register(char *p_module, set_config_callback _cfg_callback)
{
    aic_ctrl_t *ctrl = (aic_ctrl_t *)&s_aic_ctrl;
    aic_ctrl_cfg_cb_t *cfg_cb = NULL;

    if (strlen(p_module) > AIC_CTRL_CFG_MODULE_LEN) {
        aicx_error(0x00e4c5, "d", "module_len need less than %d \r\n", AIC_CTRL_CFG_MODULE_LEN);
        return (int32_t)NULL;
    }
    aicx_info(0x1e1961, "ss", "[%s]aic_ctrl_set_config_register module %s \r\n",__func__, p_module);

    cfg_cb = aos_malloc(sizeof(aic_ctrl_cfg_cb_t));
    if (!cfg_cb)
        return (int32_t)NULL;

    memset(cfg_cb, 0, sizeof(aic_ctrl_cfg_cb_t));
    memcpy(&cfg_cb->module, p_module, strlen(p_module));
    cfg_cb->cfg_callback = (void*)_cfg_callback;
    list_add(&cfg_cb->next, &ctrl->cfg_cb_list);

    return (int32_t)cfg_cb;
}

int32_t aic_ctrl_set_config_unregister(int32_t client_handle)
{
    aic_ctrl_t *ctrl = (aic_ctrl_t *)&s_aic_ctrl;
    aic_ctrl_cfg_cb_t *pos, *temp, *cfg_cb = NULL;
    struct list_head *head;
    int ret = AIC_EPERM;

    cfg_cb = (aic_ctrl_cfg_cb_t *)client_handle;

    head = &ctrl->cfg_cb_list;

    /* find cb, than del it */
    list_for_each_entry_safe(pos, temp, head, next, aic_ctrl_cfg_cb_t) {
        if (cfg_cb == pos->cfg_callback) {
            list_del(&pos->next);
            aos_free(pos);
            ret = AIC_SUCCESS;
            break;
        }
    }

    return ret;
}

static int32_t aic_ctrl_set_config_notify(aic_ctrl_cfg_t *p_cfg_info)
{
    aic_ctrl_t *ctrl = (aic_ctrl_t *)&s_aic_ctrl;
    aic_ctrl_cfg_cb_t *pos;
    set_config_callback cfg_cb_fun = NULL;
    struct list_head *head;
    int ret = AIC_EPERM;

    aicx_info(0x0d20b8, "", "aic_ctrl_set_config_notify enter.\n");

    head = &ctrl->cfg_cb_list;

    /* find the handler to process it. */
    list_for_each_entry(pos, head, next, aic_ctrl_cfg_cb_t) {
        if (0 == strcmp(p_cfg_info->module, pos->module)) {
            aicx_info(0x0ec8a7, "s", "aic ctrl set config notify %s.\n", pos->module);
            cfg_cb_fun = (set_config_callback)pos->cfg_callback;
            if (NULL != cfg_cb_fun)
                ret = cfg_cb_fun(&p_cfg_info->cfg_data, sizeof(p_cfg_info->cfg_data));
            break;
        }
    }

    return ret;
}

void aic_ctrl_set_abnormal_pin(bool active)
{
    struct ext_modem_abnormal_status *abnl = ext_modem_get_abnormal_status_cfg();
    u32 gpio_abnl = abnl->abnormal_status_n;

    if (gpio_abnl > 0) {
        aic_gpio_request(gpio_abnl);
        aic_gpio_output(gpio_abnl, active ? GPIO_OUTPUT_LVL_LOW : GPIO_OUTPUT_LVL_HIGH);
    }
}

void aic_ctrl_str_notify(const char *str)
{
    aic_ctrl_t *ctrl = (aic_ctrl_t *)&s_aic_ctrl;
    int ret, len;
    char *msg;

    if (aic_sys_is_assert()) {
        if (ctrl->ctrl_mux)
            aic_cmux_write(ctrl->ctrl_mux, str, strlen(str), AOS_NO_WAIT);
        return;
    }

    if (!ctrl->send_queue)
        return;

    len = strlen(str);
    msg = (char *)aos_malloc(len + 1);
    AIC_ASSERT(msg);
    strcpy(msg, str);

    ret = aos_send_to_queue(ctrl->send_queue, (void*)&msg, AOS_NO_WAIT);
    if (ret != AOS_SUCCESS) {
        aicx_info(0x1249fa, "ds", "aic ctrl notify err(%d), str is %s.\n", ret, str);
    }
}

void aic_ctrl_state_notify(AIC_STATE state)
{
    char *str_state = NULL;

    aicx_info(0x095708, "d", "aic ctrl notify state=%d.\n", state);

    switch (state) {
        case AIC_BOOTED_STATE:
            str_state = AIC_CTRL_BOOTED;
            break;

        case AIC_ALIVE_STATE:
            str_state = AIC_CTRL_ALIVE;
            break;

        case AIC_ASSERT_STATE:
            str_state = AIC_CTRL_ASSERT;
            break;

        case AIC_SHUTDOWN_STATE:
            str_state = AIC_CTRL_SHUTDOWN;
            break;

        default:
            return;
    }

    aic_ctrl_str_notify(str_state);
}

void aic_ctrl_mode_notify(BOOT_MODE_E mode)
{
    const char *str_mode = NULL;

    aicx_info(0x1b3f49, "d", "aic ctrl notify mode=%d.\n", mode);

    switch (mode) {
        case BOOT_MODE_NORMAL:
            str_mode = AIC_CTRL_NORMAL;
            break;

        case BOOT_MODE_CHARGE:
            str_mode = AIC_CTRL_CHG;
            break;

        case BOOT_MODE_BASIC:
            str_mode = AIC_CTRL_BAISC;
            break;

        case BOOT_MODE_GXEM:
            str_mode = AIC_CTRL_ENG;
            break;

        default:
            return;
    }

    aic_ctrl_str_notify(str_mode);
}


void aic_ctrl_cp_state_recv(subsys_state_t state, void *p)
{
    if (state == SUBSYS_STATE_ALIVE) {
        aic_ctrl_state_notify(AIC_ALIVE_STATE);
    } else if (state == SUBSYS_STATE_ASSERT) {
#if 0
        /* cp assert, ap also assert. */
        //AIC_ASSERT(0);
#else
        /* send a cmd to cp for output the assertinfo. */
        ap_send_cmd_to_cp(DIAG_CMD_ASSERT, ASSERT_ALLINFO_SUBCMD);

        if (adbg_is_cp_dump()) {
            /* send cmd to cp for dump cp. */
            ap_send_cmd_to_cp(DIAG_CMD_ASSERT, ASSERT_DUMP_SUBCMD);
        }
#endif
    }
}

static void aic_ctrl_send_thread(void *p)
{
    aic_ctrl_t *ctrl = (aic_ctrl_t *)p;
    aos_status ret;
    size_t len;
    char *str;

    aicx_info(0x03c70f, "", "enter aic ctrl send thread.\n");

#ifdef CONFIG_AIC_CTRL_D2D
    /* check if unexpect reset, assert to notify ap. */
    if (aic_boot_mode_is_no_sw_no_pwr()) {
        /* wait cmux ready. */
        aos_sleep(2000);
        AIC_ASSERT(0);
    }
#endif

    while (TRUE) {
        ret = aos_recv_from_queue(ctrl->send_queue, (void*)&str, AOS_WAIT_FOREVER);
        if (ret == AOS_SUCCESS){
            aicx_info(0x0af0e1, "s", "aic ctrl recv notify str=%s.\n", str);
            len = aic_cmux_write(ctrl->ctrl_mux, str, strlen(str), AOS_WAIT_FOREVER);
            aos_free(str);
            aicx_info(0x098631, "d", "aic ctrl cmux write len=%d.\n", len);
        } else {
            aos_sleep(100);
        }
    }
}

void aic_ctrl_recv_thread(void *p)
{
    aic_ctrl_t *ctrl = (aic_ctrl_t *)p;
    char buf[AIC_CTRL_CFG_BUFFER_LEN+1] = {0};
    size_t size;

    aicx_info(0x03a36d, "", "enter aic ctrl recv thread.\n");

    while (1) {
        size = aic_cmux_read(ctrl->ctrl_mux, buf, sizeof(buf) - 1, -1);
        AIC_ASSERT(size < sizeof(buf));
        buf[size] = 0;
        aicx_info(0x1b3e63, "s", "aic ctrl mux read %s.\n", buf);

        if (0 == strcmp(buf, AIC_CTRL_SHUTDOWN)) {
            sysoff_shutdown();
        } else if (0 == strcmp(buf, AIC_CTRL_ASSERT)) {
            AICX_ASSERT_IFX(0, 0x1884c0, "", "ext assert.");
        }
#ifdef CONFIG_AIC_CTR_DBG_ON_CMUX
        else if (strcasestr(buf, AIC_CTRL_INTER_MODEM_DBG)) {
            char *value;
            u8 old_dev;
            bool on;

            aicx_info(0x14bcc4, "s", "aic ctrl dbg %s.\n", buf);

            value = buf + strlen(AIC_CTRL_INTER_MODEM_DBG) + 1;
            if (0 == strcmp(value, AIC_CTRL_STR_ON))
                on = TRUE;
            else if (0 == strcmp(value, AIC_CTRL_STR_OFF))
                on = FALSE;
            else
                break;

            old_dev = adbg_get_log_dev();
            if (on) {
                if (old_dev != AIC_IODEV_CMUX) {
                    adbg_set_log_bak_dev(old_dev);
                    adbg_set_log_dev((u8)AIC_IODEV_CMUX);
                    alog_set_output_dev((u8)AIC_IODEV_CMUX);
                    adbg_set_ldump_on(TRUE);
                    alog_set_cp_ldump(TRUE);
                }
            } else {
                if (old_dev == AIC_IODEV_CMUX) {
                    old_dev = adbg_get_log_bak_dev();
                    adbg_set_log_dev((u8)old_dev);
                    alog_set_output_dev((u8)old_dev);
                } else {
                    adbg_set_log_bak_dev(old_dev);
                }
                adbg_set_ldump_on(FALSE);
                alog_set_cp_ldump(FALSE);
            }
        } else if (strcasestr(buf, AIC_CTRL_INTER_CPLOG)) {
            char *value;
            u8 sw;

            aicx_info(0x048152, "s", "aic ctrl cplog %s.\n", buf);

            value = buf + strlen(AIC_CTRL_INTER_CPLOG) + 1;
            if (0 == strcmp(value, AIC_CTRL_STR_ON))
                sw = ALOG_ENA_ALL;
            else if (0 == strcmp(value, AIC_CTRL_STR_OFF))
                sw = ALOG_DIS_ALL;
            else
                break;

            alog_set_cp_log_switch(sw);
        }
#endif
        else if (strcasestr(buf, AIC_CTRL_SET_CONFIG)) {
            aic_ctrl_cfg_t *p_cfg = NULL;
            char *cfg_data = NULL;
            int ret = AIC_EPERM;
            aicx_info(0x0242e3, "s", "aic ctrl set config %s.\n", buf);

            cfg_data = buf + strlen(AIC_CTRL_SET_CONFIG);

            p_cfg = (aic_ctrl_cfg_t *)cfg_data;

            ret = aic_ctrl_set_config_notify(p_cfg);
            if (AIC_SUCCESS != ret) {
                aicx_info(0x0bfe63, "", "aic ctrl set config failed.\n");
            }
            /* TODO: send set config ack to MCU */
        }
    }
}

int aic_ctrl_init(void)
{
    aic_ctrl_t *ctrl = (aic_ctrl_t *)&s_aic_ctrl;

    aic_ctrl_set_abnormal_pin(FALSE);

    subsys_add_state_notify(AIC_SYS_CP, aic_ctrl_cp_state_recv, NULL);

    list_head_init(&(ctrl->cfg_cb_list));

    ctrl->cfg_mutex = aos_create_mutex("aic_ctr_cfg");
    AIC_ASSERT(ctrl->cfg_mutex);

    ctrl->ctrl_mux = aic_cmux_open(AIC_CMUX_MCTRL_COM);
    AIC_ASSERT(ctrl->ctrl_mux);

    ctrl->send_queue = aos_create_queue("aic_ctr_send", sizeof(void*), 10);
    AIC_ASSERT(ctrl->send_queue);

    ctrl->send_thread =
        aos_create_thread_simply("aic_ctr_send", aic_ctrl_send_thread, ctrl,
                                 AIC_CTRL_STACK_SIZE, AIC_CTRL_THREAD_PRI);
    AIC_ASSERT(ctrl->send_thread);

    ctrl->recv_thread =
        aos_create_thread_simply("aic_ctr_recv", aic_ctrl_recv_thread, ctrl,
                                 AIC_CTRL_STACK_SIZE, AIC_CTRL_THREAD_PRI);
    AIC_ASSERT(ctrl->recv_thread);

    /* notify the boot mode. */
    aic_ctrl_mode_notify(aic_boot_mode_get());

    /* notify the boot state. */
    aic_ctrl_state_notify(AIC_BOOTED_STATE);

    return AIC_SUCCESS;
}
AIC_COMP_P2_INIT_EXPORT(aic_ctrl_init);

