/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is adc blt at cmd handle.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x105d58
#define LOG_MODULE    AMOD_PMIC
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aic_at_resp_handle.h>
#include <aic_bitmap.h>
#include <adc.h>
#include <timer_inc.h>
#include <adc_cus_cfg.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

static u32 at_adc_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^ADCTEST");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "a,b,c(a:adc channel, b:expected value c:tolerance)");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_adc_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    ulong rsp[2];
    char rsp_str[30] = {0};
    int ret = 0;
    u32 chan, experted, tolerance;
    u32 adc_val;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^ADCTEST=", &count);
    if (!paras_arr || count != 3) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    chan = atoi((u8 *)paras_arr[0]);
    experted = atoi((u8 *)paras_arr[1]);
    tolerance = atoi((u8 *)paras_arr[2]);


    ret = adc_chan_voltage_get(chan, &adc_val);
    rsp[0] = (ulong)rsp_str;
    if (ret) {
        strcpy(rsp_str, "ERROR");
        rsp[1] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (abs((int)adc_val - (int)experted) < tolerance) {
        sprintf(rsp_str, "PASS,%d", adc_val);
        rsp[1] = (ulong)AT_RESULT_OK;
    } else {
        sprintf(rsp_str, "FAIL,%d", adc_val);
        rsp[1] = (ulong)AT_RESULT_OK;
    }

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

AT_CMD_HANDLE adc_at_handles[] = {
    {
        "AT^ADCTEST",
        at_adc_test_cmd,
        AIC_NULL,
        at_adc_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};

static u32 at_cm_adc_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+MADC: (0-%d)", ADC_CHAN_MAX);
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
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

static u32 at_cm_adc_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    u16 count;
    int ret, channel;
    u32 chan_adc_val;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+MADC=", &count);
    if (!paras_arr || count != 1)
        goto CMD_ERROR;

    if(!at_cm_is_all_digit((char * )paras_arr[0]))
        goto CMD_ERROR;

    channel = atoi((u8 *)paras_arr[0]);
    if (channel > ADC_CHAN_MAX)
        goto CMD_ERROR;

    ret = adc_chan_voltage_get(channel, &chan_adc_val);
    if (ret)
        goto CMD_ERROR;

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MADC: %d", chan_adc_val);
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

static u32 at_mchipinfo_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+MCHIPINFO: (\"vbat\",\"temp\")");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_mchipinfo_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    u16 count;
    u32 val;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+MCHIPINFO=", &count);
    if (!paras_arr || count != 1)
        goto CMD_ERROR;

    if (!strcmp((char *)paras_arr[0],"VBAT") || !strcmp((char *)paras_arr[0],"vbat")) {
        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MCHIPINFO: \"vbat\",%d", vbat_vol_mv_get());
    } else if (!strcmp((char *)paras_arr[0],"TEMP") || !strcmp((char *)paras_arr[0],"temp")) {
        val = AIC_READ32(REG_BASE_TOP_THMC + 0x14);
        if (!val)
            goto CMD_ERROR;
        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MCHIPINFO: \"temp\",%d.%d", (val/2 -273), val%2?5:0);
    } else
        goto CMD_ERROR;

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

AT_CMD_HANDLE cm_adc_at_handles[] = {
    {
        "AT+MADC",
        at_cm_adc_test_cmd,
        AIC_NULL,
        at_cm_adc_cmd,
        AIC_NULL
    },
    {
        "AT+MCHIPINFO",
        at_mchipinfo_test_cmd,
        AIC_NULL,
        at_mchipinfo_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};

int adc_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&adc_at_handles[0]);
    aic_add_at_cmd_handle(&cm_adc_at_handles[0]);

    return 0;
}
AIC_COMP_L2_INIT_EXPORT(adc_at_cmd_init);
