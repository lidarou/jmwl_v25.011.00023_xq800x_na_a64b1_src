/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is power supply at cmd handle.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04ed52
#define LOG_MODULE    AMOD_PMIC
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_at_resp_handle.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <power_supply.h>
#include <adbg_cfg_api.h>
#include <diag_cmd_api.h>

#define POWER_MAX_AT_RESP_LENGTH        64UL
#define POWER_REPLY_LINE_COUNT_ONCE     2UL

u32 aic_chg_on_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[POWER_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[POWER_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, POWER_MAX_AT_RESP_LENGTH, "+CHGON");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 aic_chg_off_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[POWER_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[POWER_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, POWER_MAX_AT_RESP_LENGTH, "+CHGOFF");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

void aic_at_chg_on(void)
{
    psy_charging_enable(true);
    aic_battery_monitor_start();
}

void aic_at_chg_off(void)
{
    aic_battery_monitor_stop();
    psy_charging_enable(false);
}

u32 aic_power_chgon_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong line_pointers[POWER_REPLY_LINE_COUNT_ONCE];
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    aic_at_chg_on();
    strcpy(result_line0, "OK");

    adbg_set_chg_pd(0);

    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

u32 aic_power_chgoff_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong line_pointers[POWER_REPLY_LINE_COUNT_ONCE];
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    aic_at_chg_off();
    strcpy(result_line0, "OK");

    adbg_set_chg_pd(1);

    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

AT_CMD_HANDLE power_at_handles[] = {
    {"AT+CHGON", aic_chg_on_test_cmd, AIC_NULL,
     aic_power_chgon_cmd, AIC_NULL},
    {"AT+CHGOFF", aic_chg_off_test_cmd, AIC_NULL,
     aic_power_chgoff_cmd, AIC_NULL},
    {AIC_NULL},
};

int power_at_cmd_init(void)
{
    if (aic_boot_mode_is_gxem())
        return 0;

    aic_add_at_cmd_handle(&power_at_handles[0]);

    if (adbg_is_chg_pd())
        aic_at_chg_off();

    return 0;
}
AIC_COMP_L2_INIT_EXPORT(power_at_cmd_init);

void power_supply_diag_chg_off(diag_send_cmd_t *diag_cmd, void *pri)
{
    diag_rsp_cmd_t rsp;

    aic_at_chg_off();
    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, 0, diag_cmd->sys,
                 DIAG_CMD_CHG_OFF, diag_cmd->subcmd, NULL, 0);
    diag_cmd_response(&rsp);
}

int diag_power_supply_cmd_init(void)
{
    /* register power_supply diag cmd handler. */
    diag_register_cmd_handler(DIAG_CMD_CHG_OFF, power_supply_diag_chg_off, NULL);

    return 0;
}
AIC_COMP_P2_INIT_EXPORT(diag_power_supply_cmd_init);

#ifdef CONFIG_CUSTOMER_CM_AT
#define isdigit(c) ((c) >= '0' && (c) <= '9')
static u32 at_mlpdt_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+MLPDT: <0-1>,[<1-9999>]");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req_res_ptr, rsp, 2);
}

static u32 at_mlpdt_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct psy_bat_mon_info p_bat_mon;

    psy_bat_mon_state_get(&p_bat_mon);

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MLPDT: %d,%d", p_bat_mon.bat_mon_enabled, p_bat_mon.bat_mon_cycle_ms / 1000);

    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    /* Fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

static bool at_cm_is_all_digit(const char * str)
{
    u8 len = 0;
    u8 i = 0;

    len = strlen(str);
    for (i = 0; i < len; i++) {
        if(!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

static u32 at_mlpdt_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    struct psy_bat_mon_info pbat_mon;

    psy_bat_mon_state_get(&pbat_mon);

    if (pbat_mon.bat_mon_enabled)
        aic_at_chg_off();
    else
        aic_at_chg_on();

    psy_bat_mon_state_get(&pbat_mon);

    sprintf(rsp_str, "+MLPDT: %d,%d", pbat_mon.bat_mon_enabled, pbat_mon.bat_mon_cycle_ms / 1000);
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req_res_ptr, rsp, 2);
}

static u32 at_mlpdt_ctrl_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    u16 count;
    u32 enable, cycle_s;
    struct psy_bat_mon_info pbat_mon;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+MLPDT=", &count);
    if (count == 0) {
        return at_mlpdt_exe_cmd(at_req_res_ptr);
    }

    if (!paras_arr || ((count != 1) && (count != 2)))
        goto CMD_ERROR;

    if(!at_cm_is_all_digit((char * )paras_arr[0]))
        goto CMD_ERROR;

    enable = atoi((u8 *)paras_arr[0]);
    if (enable > 1)
        goto CMD_ERROR;

    if (count == 1) {
        psy_bat_mon_state_get(&pbat_mon);
        cycle_s = pbat_mon.bat_mon_cycle_ms / 1000;
    } else {
        if(!at_cm_is_all_digit((char * )paras_arr[1]))
            goto CMD_ERROR;

        cycle_s = atoi((u8 *)paras_arr[1]);
        if (cycle_s < 1 || cycle_s > 9999)
            goto CMD_ERROR;

        psy_bat_monitor_cycle_set(cycle_s * 1000);
    }

    if (enable)
        aic_at_chg_on();
    else
        aic_at_chg_off();

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MLPDT: %d,%d", enable, cycle_s);
    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    /* Fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_CMD);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

AT_CMD_HANDLE psy_cm_at_handles[] = {
    {
        "AT+MLPDT",
        at_mlpdt_test_cmd,
        at_mlpdt_query_cmd,
        at_mlpdt_ctrl_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};

int psy_cm_at_cmd_init(void)
{
    aic_at_chg_off();
    aic_add_at_cmd_handle(&psy_cm_at_handles[0]);

    return 0;
}
AIC_NORMAL_COMP_L2_INIT_EXPORT(psy_cm_at_cmd_init);
#endif

