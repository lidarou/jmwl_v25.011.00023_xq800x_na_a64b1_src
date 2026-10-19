/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This log diag cmd realize file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f8f57
#define LOG_MODULE    AMOD_ALOG

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <diag_cmd_api.h>
#include <aic_assert.h>
#include <alog_cfg.h>
#include <alog_trace_filter.h>
#include <alog_diag_cmd.h>
#include <adbg_cfg_api.h>

#ifdef CONFIG_SUBSYS_AP
aos_semaphore_ptr g_wait_cp_rsp_sem;
diag_rsp_cmd_t g_rsp_cmd;

static void alog_send_diag_cmd(u8 subcmd, void *data, u16 data_len)
{
    aos_status ret;

    ap_send_diag_cmd_to_cp(DIAG_CMD_LOG, subcmd, data, data_len);
    ret = aos_get_semaphore(g_wait_cp_rsp_sem, 3000);
    if (!ret)
        return;

    diag_set_cmd(&g_rsp_cmd, 0, ret, 1, AIC_SYS_CP, DIAG_CMD_LOG, subcmd, NULL, 0);
}

int alog_cmd_get_cp_data(u8 subcmd, u32 *value)
{
    alog_send_diag_cmd(subcmd, NULL, 0);

    if (g_rsp_cmd.err_no || !g_rsp_cmd.data)
        return g_rsp_cmd.err_no;

    *value = (*(u32 *)(g_rsp_cmd.data));
    aos_free(g_rsp_cmd.data);
    g_rsp_cmd.data = NULL;

    return AIC_SUCCESS;
}

int alog_cmd_set_cp_data(u8 subcmd, u32 value)
{
    u32 data;

    data = value;
    alog_send_diag_cmd(subcmd, &data, sizeof(data));

    return g_rsp_cmd.err_no;
}

int alog_get_cp_log_level(u8 module, u8 *value)
{
    u32 data;

    data = AMOD_CNT;
    alog_send_diag_cmd(LOG_SUBCMD_GET_LOG_LEVEL, &data, sizeof(data));

    if (g_rsp_cmd.err_no || !g_rsp_cmd.data)
        return g_rsp_cmd.err_no;

    *value = (u8)(*(u32 *)(g_rsp_cmd.data));
    aos_free(g_rsp_cmd.data);
    g_rsp_cmd.data = NULL;

    return AIC_SUCCESS;
}

int alog_set_cp_log_level(u8 module, u8 level)
{
    u32 data[2];

    data[0] = module;
    data[1] = level;
    alog_send_diag_cmd(LOG_SUBCMD_SET_LOG_LEVEL, data, sizeof(data));

    return g_rsp_cmd.err_no;
}

void alog_diag_rsp_handle(diag_recv_cmd_t *diag_cmd, void *pri)
{
    if (g_rsp_cmd.data)
        aos_free(g_rsp_cmd.data);

    diag_set_cmd(&g_rsp_cmd, diag_cmd->sn, diag_cmd->err_no, diag_cmd->m2,
                 diag_cmd->sys, diag_cmd->cmd, diag_cmd->subcmd,
                 NULL, diag_cmd->data_len);

    if (diag_cmd->data_len) {
        g_rsp_cmd.data = aos_malloc(diag_cmd->data_len);
        memcpy(g_rsp_cmd.data, diag_cmd->data, diag_cmd->data_len);
    }

    aos_put_semaphore(g_wait_cp_rsp_sem);
}

#ifdef CONFIG_DBG_CPPC_SAMP
int atdbg_cp_samppc_set(u8 act, u32 us)
{
    u32 data[2];

    data[0] = act;
    data[1] = us;
    alog_send_diag_cmd(LOG_SUBCMD_SET_PC_SAMP, data, sizeof(data));

    return g_rsp_cmd.err_no;
}
#endif
#else
void alog_diag_cmd_handle(diag_recv_cmd_t *diag_cmd, void *pri)
{
    diag_rsp_cmd_t rsp;
    void *data = NULL;
    int data_len = 0;
    int ret = DIAG_RSP_SUCC;
    u32 value = 0;
    u32 p1 = 0, p2 = 0;
    u32 *param = diag_cmd->data;

    if (diag_cmd->data_len == sizeof(u32)) {
        p1 = param[0];
    } else if (diag_cmd->data_len == 2*sizeof(u32)) {
        p1 = param[0];
        p2 = param[1];
    }

    aicx_debug(0x0d8fc1, "ddd", "alog subcmd(%d), p1=%d, p2=%d.\n", diag_cmd->subcmd, p1, p2);

    switch (diag_cmd->subcmd) {
        case LOG_SUBCMD_GET_DBG_SWITCH:
            value = adbg_get_sys_debug_ex();
            data = &value;
            data_len = sizeof(u32);
            break;

        case LOG_SUBCMD_GET_LOG_SWITCH:
            value = alog_get_log_switch_ex();
            data = &value;
            data_len = sizeof(u32);
            break;

        case LOG_SUBCMD_GET_LOG_DEV:
            value = adbg_get_log_dev();
            data = &value;
            data_len = sizeof(u32);
            break;

        case LOG_SUBCMD_GET_LOG_BAUD_RATE:
            value = adbg_get_log_baudrate();
            data = &value;
            data_len = sizeof(u32);
            break;

        case LOG_SUBCMD_GET_LOG_LEVEL:
            value = alog_get_log_level((u8)p1);
            data = &value;
            data_len = sizeof(u32);
            break;

        case LOG_SUBCMD_SET_DBG_SWITCH:
            adbg_set_sys_debug_ex(p1);
            break;

        case LOG_SUBCMD_SET_LOG_SWITCH:
            alog_set_log_switch_ex((u8)p1);
            break;

        case LOG_SUBCMD_SET_LOG_DEV:
            adbg_set_log_dev(p1);
            break;

        case LOG_SUBCMD_SET_LOG_BAUD_RATE:
            adbg_set_log_baudrate(p1);
            break;

        case LOG_SUBCMD_SET_LOG_LEVEL:
            alog_set_log_level((u8)p1, (u8)p2);
            break;

        case LOG_SUBCMD_SET_LOG_DUMP:
            adbg_set_ldump_on((u8)p1);
            break;

#ifdef CONFIG_DBG_CPPC_SAMP
        case LOG_SUBCMD_SET_PC_SAMP:
            cpu_pc_samp(p1, p2);
            break;
#endif

        default:
        break;
    }

    diag_set_rsp(&rsp, diag_cmd->sn, ret, diag_cmd->m2, diag_cmd->sys,
                 DIAG_CMD_LOG, diag_cmd->subcmd, data, data_len);

    diag_cmd_response(&rsp);
}
#endif

int alog_diag_cmd_init(void)
{
#ifdef CONFIG_SUBSYS_AP
    diag_register_cmd_handler(DIAG_CMD_LOG, alog_diag_rsp_handle, NULL);
    g_wait_cp_rsp_sem = aos_create_semaphore_simply("lgdg");
    AIC_DUMMY_ASSERT(g_wait_cp_rsp_sem);

#else
    diag_register_cmd_handler(DIAG_CMD_LOG, alog_diag_cmd_handle, NULL);
#endif
    return 0;
}
AIC_COMP_P2_INIT_EXPORT(alog_diag_cmd_init);

