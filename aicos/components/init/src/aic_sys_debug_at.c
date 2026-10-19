
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
#define AIC_THIS_FILE    0x0b426c
#define LOG_MODULE    AMOD_INIT

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_io_dev.h>
#include <aic_osa.h>
#include <aipc_api.h>
#include <aic_at_resp_handle.h>
#include <aic_boot_mode.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_sys.h>
#include <alog_cfg.h>
#include <alog_trace_filter.h>
#include <alog_diag_cmd.h>
#include <adbg_cfg_api.h>
#include <assert_adpter.h>
#include <alog_init.h>
#include <connmgr_api.h>
#include <drv_file.h>
#include <sys_mem_api.h>
#include <sysoff.h>
#include <cmux.h>

/*
 * CONFIG_BASIC_API_TEST
 * 1: enable basic mode api test function.
 */
#ifdef CONFIG_FOTA_SUPPORT
#define CONFIG_BASIC_API_TEST   1
#else
#define CONFIG_BASIC_API_TEST   0
#endif

/* *********************************************************************
 * AT^LOG cmd start
 ***********************************************************************/
static u32 atdbg_log_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};

    sprintf(rsp_str, "^LOG:0x%x", alog_get_log_switch_ex());
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_log_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    char * param;
    u16 count;
    unsigned long value;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LOG=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    param = (char *)paras_arr[0];
    value = strtoul(param, NULL, 16);

    /* only like 0x01, use the value. */
    if (param[0] != '0' || param[1] != 'x' || !param[3]) {
        value = value > 0 ? ALOG_ENA_ALL : ALOG_DIS_ALL;
    }

    alog_set_log_switch_ex((u8)value);
    rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^LOGDEV cmd start
 ***********************************************************************/
static u32 atdbg_logdev_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};

    sprintf(rsp_str, "^LOGDEV:%d", adbg_get_log_dev());
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_logdev_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LOGDEV=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);

    /* basic mode only can support uart and none. */
    if (value >= AIC_IODEV_CNT ||
        (aic_boot_mode_is_basic() && (u8)value > AIC_IODEV_UART))
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else {
        u8 old_dev = adbg_get_log_dev();

        if (old_dev != (u8)value) {
#ifdef CONFIG_ALOG_STORE_FS
            if (value == AIC_IODEV_FILE) {
                alog_store_init();
            } else {
                if (old_dev == AIC_IODEV_FILE)
                    alog_store_deinit();
            }
#endif
            adbg_set_log_dev((u8)value);
            alog_set_output_dev((u8)value);
        }
        rsp[0] = (ulong)AT_RESULT_OK;
    }

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^DIAGDEV cmd start
 ***********************************************************************/
static u32 atdbg_diagdev_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};

    sprintf(rsp_str, "^DIAGDEV:%d", adbg_get_diag_dev());
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_diagdev_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^DIAGDEV=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);

    /* basic mode only can support uart. */
    if ((value != AIC_IODEV_USB && value != AIC_IODEV_UART) ||
        (aic_boot_mode_is_basic() && value == AIC_IODEV_USB))
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else {
        if ((u8)value != adbg_get_diag_dev()) {
            adbg_set_diag_dev(value);
            diag_cmd_agency_reset_iodev();
        }
        rsp[0] = (ulong)AT_RESULT_OK;
    }

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^LOGBR cmd start
 ***********************************************************************/
static u32 atdbg_logbr_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    u32 value;
    int len;

    value = adbg_get_log_baudrate();
    strcpy(rsp_str, "^LOGBR");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "bandrate=%d", value);

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_logbr_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LOGBR=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);
    if (value < 115200 || value > 12000000) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    adbg_set_log_baudrate(value);
    conn_data_set_baud_rate(CONN_DATA_LOG, value);
    rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^DIAGBR cmd start
 ***********************************************************************/
static u32 atdbg_diagbr_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    u32 value;
    int len;

    value = adbg_get_diag_baudrate();
    strcpy(rsp_str, "^DIAGBR");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "bandrate=%d", value);

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_diagbr_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^DIAGBR=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);
    if (value < 115200 || value > 12000000) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    adbg_set_diag_baudrate(value);
    conn_data_set_baud_rate(CONN_DATA_DIAG, value);
    rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^LOGLEVEL cmd start
 ***********************************************************************/
static u32 atdbg_loglevel_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    int len;

    strcpy(rsp_str, "^LOGLEVEL");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "%d", alog_get_log_level(AMOD_CNT));
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_loglevel_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    u8 mod;
    unsigned long value;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LOGLEVEL=", &count);
    if (!paras_arr || (count != 1 && count != 2)) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);

    if (value > ALOG_LV_DEBUG)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else {
        if (count == 1)
            mod = AMOD_CNT;
        else {
            mod = aic_get_moudule((char *)paras_arr[1]);
            if (mod == AMOD_CNT) {
                rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
                goto AT_REPLY;
            }
        }

        alog_set_log_level(mod, value);
        rsp[0] = (ulong)AT_RESULT_OK;
    }

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^CPLOG cmd start
 ***********************************************************************/
static u32 atdbg_cp_log_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    int len, ret;
    u32 value;

    ret = alog_get_cp_log_switch(&value);
    rsp[0] = (ulong)rsp_str;

    strcpy(rsp_str, "^CPLOG");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    if (ret) {
        sprintf(rsp_str + len, "err=%d", ret);
        rsp[1] = (ulong)AT_SIMPLE_RESULT_ERROR;
    } else {
        sprintf(rsp_str + len, "0x%x", value);
        rsp[1] = (ulong)AT_RESULT_OK;
    }

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_cp_log_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    ulong *paras_arr;
    char *param;
    u16 count;
    unsigned long value;
    int ret, len;

    rsp[0] = (ulong)rsp_str;
    strcpy(rsp_str, "^CPLOG");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^CPLOG=", &count);
    if (!paras_arr || count != 1) {
        ret = AT_ERROR_PARA_NOT_SUPPORT;
        goto AT_REPLY;
    }

    param = (char *)paras_arr[0];
    value = strtoul(param, NULL, 16);

    /* only head with 0x, use the value. */
    if (param[0] != '0' || param[1] != 'x') {
       if (value > 1) {
           /* only support 0 or 1 */
           ret = AT_ERROR_PARA_NOT_SUPPORT;
           goto AT_REPLY;
        }
       value =  value > 0 ? ALOG_ENA_ALL : ALOG_DIS_ALL;
    } 

    ret = alog_set_cp_log_switch((u8)value);

AT_REPLY:
    if (ret) {
        sprintf(rsp_str + len, "err=%d", ret);
        rsp[1] = (ulong)AT_SIMPLE_RESULT_ERROR;
    } else {
        sprintf(rsp_str + len, "%ld", value);
        rsp[1] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^CPDELAY cmd start
 ***********************************************************************/
#if 0
static u32 atdbg_cp_delay_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    u8 value;
    int len;

    value = adbg_is_cp_delay_boot();
    rsp[0] = (ulong)rsp_str;
    strcpy(rsp_str, "^CPDELAY");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "%d", value);
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}
#endif

static u32 atdbg_cp_delay_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^CPDELAY=", &count);

    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_ERROR_PARA_NOT_SUPPORT;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);
    adbg_set_cp_delay_boot(value);
    rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^CPLOGLEVEL cmd start
 ***********************************************************************/
static u32 atdbg_cp_loglevel_query_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[30] = {0};
    ulong rsp[2];
    int len, ret;
    u8 value;

    ret = alog_get_cp_log_level(AMOD_CNT, &value);
    rsp[0] = (ulong)rsp_str;

    strcpy(rsp_str, "^CPLOGLEVEL");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';

    if (ret) {
        sprintf(rsp_str + len, "err=%d", ret);
        rsp[1] = (ulong)AT_SIMPLE_RESULT_ERROR;
    } else {
        sprintf(rsp_str + len, "%d", value);
        rsp[1] = (ulong)AT_RESULT_OK;
    }

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_cp_loglevel_set_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[30] = {0};
    unsigned long value;
    ulong *paras_arr;
    int ret, len;
    ulong rsp[2];
    u16 count;
    u8 mod;

    rsp[0] = (ulong)rsp_str;
    strcpy(rsp_str, "^CPLOGLEVEL");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^CPLOGLEVEL=", &count);
    if (!paras_arr || (count != 1 && count != 2)) {
        ret = AT_ERROR_PARA_NOT_SUPPORT;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);
    if (value > ALOG_LV_DEBUG) {
         ret = AIC_EINVAL;
    } else {
        if (count == 1)
            mod = AMOD_CNT;
        else {
            mod = aic_get_moudule((char *)paras_arr[1]);
            if (mod == AMOD_CNT) {
                ret = AT_ERROR_PARA_NOT_SUPPORT;
                goto AT_REPLY;
            }
        }

        ret = alog_set_cp_log_level(mod, value);
    }

AT_REPLY:
    if (ret) {
        sprintf(rsp_str + len, "err=%d", ret);
        rsp[1] = (ulong)AT_SIMPLE_RESULT_ERROR;
    } else {
        sprintf(rsp_str + len, "%ld", value);
        rsp[1] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

/* *********************************************************************
 * AT^LOGDUMP cmd start
 ***********************************************************************/
#if 0
static u32 atdbg_ldump_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    u8 value;
    int len;

    value = adbg_is_ldump_on();
    rsp[0] = (ulong)rsp_str;
    strcpy(rsp_str, "^LOGDUMP");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "%d", value);
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}
#endif

static u32 atdbg_ldump_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LOGDUMP=", &count);

    if (!paras_arr || count != 1) {
        ret = AT_ERROR_PARA_NOT_SUPPORT;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);

    ret = alog_set_cp_ldump(value);

AT_REPLY:
    if (ret) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    } else {
        adbg_set_ldump_on(value);
        rsp[0] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^SYSDUMP cmd start
 ***********************************************************************/
static u32 atdbg_sysdump_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    int len;

    strcpy(rsp_str, "^SYSDUMP");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';

#ifdef CONFIG_SUPPORT_ADSP_DUMP
    sprintf(rsp_str + len, "ap=%d, cp=%d, adsp=%d.",
            adbg_is_ap_dump(),
            adbg_is_cp_dump(),
            adbg_is_adsp_dump());
#else
    sprintf(rsp_str + len, "ap=%d, cp=%d.",
            adbg_is_ap_dump(),
            adbg_is_cp_dump());
#endif

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_sysdump_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value, sys;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^SYSDUMP=", &count);
    if (!paras_arr || count != 2) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    sys = strtoul((char *)paras_arr[0], NULL, 10);
    value = strtoul((char *)paras_arr[1], NULL, 10);

    if (value > 1 || sys >= AIC_SYS_NR)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else {
        switch (sys) {
        case AIC_SYS_AP:
            adbg_set_ap_dump(value);
            break;
        case  AIC_SYS_CP:
            adbg_set_cp_dump(value);
            break;

#ifdef CONFIG_SUPPORT_ADSP_DUMP
        case  AIC_SYS_ADSP:
            adbg_set_adsp_dump(value);
            break;
#endif

        default:
            break;
        }
        rsp[0] = (ulong)AT_RESULT_OK;
    }

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^AUTORESET cmd start
 ***********************************************************************/
static u32 atdbg_autoreset_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[40] = {0};
    int len;

    strncpy(rsp_str, "^AUTORESET", sizeof(rsp_str));
    len = strlen(rsp_str);
    rsp_str[len++] = ':';

#ifdef CONFIG_SUPPORT_ADSP_DUMP
    snprintf(rsp_str + len, sizeof(rsp_str) - len, "ap=%d, cp=%d, adsp=%d.",
            adbg_is_ap_reset(),
            adbg_is_cp_reset(),
            adbg_is_adsp_reset());
#else
    snprintf(rsp_str + len, sizeof(rsp_str) - len, "ap=%d, cp=%d.",
            adbg_is_ap_reset(),
            adbg_is_cp_reset());
#endif

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_autoreset_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value = 0, sys;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^AUTORESET=", &count);
    if (!paras_arr || count != 2) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    sys = strtoul((char *)paras_arr[0], NULL, 10);
    value = strtoul((char *)paras_arr[1], NULL, 10);

    if (value > 1 || sys >= AIC_SYS_NR)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else {
        switch (sys) {
        case AIC_SYS_AP:
            adbg_set_ap_reset(value);
            break;
        case  AIC_SYS_CP:
            adbg_set_cp_reset(value);
            break;

#ifdef CONFIG_SUPPORT_ADSP_DUMP
        case  AIC_SYS_ADSP:
            adbg_set_adsp_reset(value);
            break;
#endif

        default:
            break;
        }
        rsp[0] = (ulong)AT_RESULT_OK;
    }

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^ASSERT cmd start
 ***********************************************************************/
static u32 atdbg_assert_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];

    rsp[0] = (ulong)AT_RESULT_OK;

    AIC_ASSERT(0);
    return 0;
}

/* *********************************************************************
 * AT^ASSERTCP cmd start
 ***********************************************************************/
static u32 atdbg_assertcp_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];

    rsp[0] = (ulong)AT_RESULT_OK;

    ap_send_cmd_to_cp(DIAG_CMD_ASSERT, ASSERT_ASSERT_SUBCMD);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^RESET cmd start
 ***********************************************************************/
static u32 atdbg_reset_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;

    value = BOOT_MODE_NORMAL;
    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^RESET=", &count);
#ifdef CONFIG_FOTA_SUPPORT
    if (paras_arr && count == 1) {
        if (0 == strncasecmp((char *)paras_arr[0], "basic", 5))
            value = BOOT_MODE_BASIC;
    }
#endif

    rsp[0] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req, rsp, 1);

    aicx_debug(0x14a991, "", "at resetsys!");

#ifdef CONFIG_AIC_SYSOFF
    /* reboot sys */
    if (value == BOOT_MODE_BASIC)
        sysoff_reset_basic();
    else
        sysoff_reset();
#endif

    while (1);
    return 0;
}

#ifdef CONFIG_CUSTOMER_AT
static u32 atdbg_creset_test_cmd(AT_REQ_RESP *at_req)
{
    aic_reply_ok_resp(at_req);

    return AIC_SUCCESS;
}
#endif

#ifdef CONFIG_CP_RESET_ONLY_CP
/* *********************************************************************
 * AT^RESETCP cmd start
 ***********************************************************************/
static u32 atdbg_resetcp_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    u8 cpdump = adbg_is_cp_dump();

    rsp[0] = (ulong)AT_RESULT_OK;

    aicx_debug(0x0b7976, "", "at resetcp!");

    adbg_set_cp_dump(1);

    ap_send_cmd_to_cp(DIAG_CMD_ASSERT, ASSERT_ASSERT_SUBCMD);
    aos_sleep(1000);

    adbg_set_cp_dump(cpdump);

    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}
#endif

#ifdef CONFIG_DBG_CPPC_SAMP
/* *********************************************************************
 * AT^SAMPPC cmd start
 ***********************************************************************/
static u32 atdbg_cp_samppc_set_cmd(AT_REQ_RESP *at_req)
{
    unsigned long value = 100;
    ulong *paras_arr;
    ulong rsp[1];
    u16 count;
    int ret;
    u8 act;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^CPSAMPPC=", &count);
    if (!paras_arr  || count > 2) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    act = (u8)(strtoul((char *)paras_arr[0], NULL, 10));
    if (count > 1)
        value = strtoul((char *)paras_arr[1], NULL, 10);

    ret = atdbg_cp_samppc_set(act, value);
    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}
#endif

#if IS_CONF_ENABLED(CONFIG_BASIC_API_TEST)
/* *********************************************************************
 * AT^STOPCP cmd start
 ***********************************************************************/
static u32 atdbg_stopcp_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];

    rsp[0] = (ulong)AT_RESULT_OK;

#ifdef CONFIG_AIC_SYSOFF
    sysoff_stop_cp();
#endif

    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^BKSYS cmd start
 ***********************************************************************/
static u32 atdbg_backup_sys_diff_img_cmd(AT_REQ_RESP *at_req)
{
    addr_t addr;
    size_t size;
    ulong rsp[1];

    sys_get_backup_fota_diff_img_area(&addr, &size);
    memset((void *)addr, 0x5a, size);

    rsp[0] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^BKEXT cmd start
 ***********************************************************************/
static u32 atdbg_backup_ext_img_cmd(AT_REQ_RESP *at_req)
{
    addr_t src_addr, dst_addr;
    size_t src_size, dst_size, left, once, rd = 0;
    u8 *src, *dst;
    ulong rsp[1];

    sys_get_extend_img_actual_area(&src_addr, &src_size);
    sys_get_backup_extend_img_area(&dst_addr, &dst_size);

    src = (u8*)src_addr;
    dst = (u8*)dst_addr;
    left = src_size;

    while (left) {
        once = MIN(SIZE_64K, left);
        memcpy(dst + rd, src + rd, once);
        left -= once;
        rd += once;
    }

    rsp[0] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^RSEXT cmd start
 ***********************************************************************/
static u32 atdbg_restore_ext_img_cmd(AT_REQ_RESP *at_req)
{
    addr_t src_addr, dst_addr;
    size_t src_size, dst_size, left, once, rd = 0, wt;
    u8 *src, *dst;
    ulong rsp[2];
    int status;
    u32 handle;

    sys_get_extend_img_actual_area(&dst_addr, &dst_size);
    sys_get_backup_extend_img_area(&src_addr, &src_size);

    src = (u8*)src_addr;
    left = dst_size;
    status = fmd_open("ap_extend", &handle);
    AIC_DUMMY_ASSERT(AIC_SUCCESS == status);

    while (left) {
        once = MIN(SIZE_64K, left);
        dst = fota_ext_malloc(once);
        AIC_DUMMY_ASSERT(dst);

        memcpy(dst, src + rd, once);
        wt = fmd_write(handle, 0, once, dst);
        if (wt == once) {
            left -= once;
            rd += once;
        }
        fota_ext_free(dst);
    }

    fmd_close(handle);

    rsp[0] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}
#endif /* CONFIG_BASIC_API_TEST */

/* *********************************************************************
 * AT^DBGGATE cmd start
 ***********************************************************************/
static u32 atdbg_dbggate_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    int len;

    strcpy(rsp_str, "^DBGGATE");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "dbg gate=0x%x", adbg_get_sys_debug_ex());

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_dbggate_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;
    char *param;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^DBGGATE=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    param = (char *)paras_arr[0];
    if (param[0] != '0' || param[1] != 'x') {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 16);
    adbg_set_sys_debug_ex((u32) value);
    rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

#ifdef CONFIG_FULL_DEBUG_AT
/* *********************************************************************
 * AT^CPDBGGATE cmd start
 ***********************************************************************/
static u32 atdbg_cpdbggate_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    u32 value;
    int len;

    alog_get_cp_dbg_gate(&value);
    strcpy(rsp_str, "^CPDBGGATE");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "dbg gate=0x%x", value);

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_cpdbggate_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;
    char *param;


    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^CPDBGGATE=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    param = (char *)paras_arr[0];
    if (param[0] != '0' || param[1] != 'x') {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 16);
    alog_set_cp_dbg_gate((u32) value);
    rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^CPLOGDEV cmd start
 ***********************************************************************/
static u32 atdbg_cplogdev_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    u32 value = 0;
    int len, ret;

    ret = alog_get_cp_log_dev(&value);
    strcpy(rsp_str, "^CPLOGDEV");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "dev=0x%x", value);

    rsp[0] = (ulong)rsp_str;
    if (ret)
        rsp[1] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_cplogdev_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^CPLOGDEV=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);
    ret = alog_set_cp_log_dev(value);
    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;


AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

/* *********************************************************************
 * AT^CPLOGBR cmd start
 ***********************************************************************/
static u32 atdbg_cplogbr_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    u32 value = 0;
    int len;
    int ret = alog_get_cp_log_baud_rate(&value);

    strcpy(rsp_str, "^CPLOGBR");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "bandrate=%d", value);

    rsp[0] = (ulong)rsp_str;
    if (ret)
        rsp[1] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_cplogbr_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;
    char *param;
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^CPLOGBR=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    param = (char *)paras_arr[0];
    value = strtoul((char *)paras_arr[0], NULL, 10);
    ret = alog_set_cp_log_baud_rate(value);

    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

#ifdef CONFIG_CMUX
/* *********************************************************************
 * AT^CMUXLOG cmd start
 ***********************************************************************/
static u32 atdbg_cmuxlog_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[30] = {0};
    int len;

    strcpy(rsp_str, "^CMUXLOG");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "cmux log flag=%d", alog_get_log_level(AMOD_CMUX));

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atdbg_cmuxlog_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    unsigned long value;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^CMUXLOG=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    value = strtoul((char *)paras_arr[0], NULL, 10);

    if (value > ALOG_LV_DEBUG)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else {
        alog_set_log_level(AMOD_CMUX, value);
        rsp[0] = (ulong)AT_RESULT_OK;
    }

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}
#endif
#endif

/*
 *  Add debug AT handle here.
 */
const static AT_CMD_HANDLE debug_at_handles[] =
{
    {
        "AT^LOG",
        NULL,
        atdbg_log_query_cmd,
        atdbg_log_set_cmd,
        AIC_NULL
    },
    {
        "AT^CPLOG",
        NULL,
        atdbg_cp_log_query_cmd,
        atdbg_cp_log_set_cmd,
        AIC_NULL
    },
    {
        "AT^LOGDEV",
        NULL,
        atdbg_logdev_query_cmd,
        atdbg_logdev_set_cmd,
        AIC_NULL
    },
    {
        "AT^DIAGDEV",
        NULL,
        atdbg_diagdev_query_cmd,
        atdbg_diagdev_set_cmd,
        AIC_NULL
    },
    {
        "AT^LOGBR",
        AIC_NULL,
        atdbg_logbr_query_cmd,
        atdbg_logbr_set_cmd,
        AIC_NULL
    },
    {
        "AT^DIAGBR",
        AIC_NULL,
        atdbg_diagbr_query_cmd,
        atdbg_diagbr_set_cmd,
        AIC_NULL
    },
    {
        "AT^SYSDUMP",
        NULL,
        atdbg_sysdump_query_cmd,
        atdbg_sysdump_set_cmd,
        AIC_NULL
    },
    {
        "AT^AUTORESET",
        NULL,
        atdbg_autoreset_query_cmd,
        atdbg_autoreset_set_cmd,
        AIC_NULL
    },
    {
        "AT^ASSERT",
        AIC_NULL,
        AIC_NULL,
        atdbg_assert_set_cmd,
        AIC_NULL
    },
    {
        "AT^ASSERTCP",
        AIC_NULL,
        AIC_NULL,
        atdbg_assertcp_set_cmd,
        AIC_NULL
    },
    {
        "AT^RESET",
        AIC_NULL,
        AIC_NULL,
        atdbg_reset_set_cmd,
        AIC_NULL
    },
    {
        "AT^CPDELAY",
        NULL,
        NULL, //atdbg_cp_delay_query_cmd,
        atdbg_cp_delay_set_cmd,
        AIC_NULL
    },
    {
        "AT^LOGDUMP",
        NULL,
        NULL,//atdbg_ldump_query_cmd,
        atdbg_ldump_set_cmd,
        AIC_NULL
    },
    {
        "AT^DBGGATE",
        AIC_NULL,
        atdbg_dbggate_query_cmd,
        atdbg_dbggate_set_cmd,
        AIC_NULL
    },
    {
        "AT^LOGLEVEL",
        NULL,
        atdbg_loglevel_query_cmd,
        atdbg_loglevel_set_cmd,
        AIC_NULL
    },
    {
        "AT^CPLOGLEVEL",
        NULL,
        atdbg_cp_loglevel_query_cmd,
        atdbg_cp_loglevel_set_cmd,
        AIC_NULL
    },
#ifdef CONFIG_FULL_DEBUG_AT
    {
        "AT^CPDBGGATE",
        AIC_NULL,
        atdbg_cpdbggate_query_cmd,
        atdbg_cpdbggate_set_cmd,
        AIC_NULL
    },
    {
        "AT^CPLOGDEV",
        AIC_NULL,
        atdbg_cplogdev_query_cmd,
        atdbg_cplogdev_set_cmd,
        AIC_NULL
    },
    {
        "AT^CPLOGBR",
        AIC_NULL,
        atdbg_cplogbr_query_cmd,
        atdbg_cplogbr_set_cmd,
        AIC_NULL
    },

#ifdef CONFIG_CMUX
    {
        "AT^CMUXLOG",
        AIC_NULL,
        atdbg_cmuxlog_query_cmd,
        atdbg_cmuxlog_set_cmd,
        AIC_NULL
    },
#endif
#endif

#ifdef CONFIG_CUSTOMER_AT
    {
        "AT+CRESET",
        atdbg_creset_test_cmd,
        AIC_NULL,
        atdbg_reset_set_cmd,
        AIC_NULL
    },
#endif

#ifdef CONFIG_CP_RESET_ONLY_CP
    {
        "AT^RESETCP",
        AIC_NULL,
        AIC_NULL,
        atdbg_resetcp_set_cmd,
        AIC_NULL
    },
#endif

#ifdef CONFIG_DBG_CPPC_SAMP
    {
        "AT^CPSAMPPC",
        AIC_NULL,
        AIC_NULL,
        atdbg_cp_samppc_set_cmd,
        AIC_NULL
    },
#endif

#if IS_CONF_ENABLED(CONFIG_BASIC_API_TEST)
    {
        "AT^STOPCP",
        AIC_NULL,
        AIC_NULL,
        atdbg_stopcp_set_cmd,
        AIC_NULL
    },
    {
        "AT^BKSYS",
        AIC_NULL,
        AIC_NULL,
        atdbg_backup_sys_diff_img_cmd,
        AIC_NULL
    },
    {
        "AT^BKEXT",
        AIC_NULL,
        AIC_NULL,
        atdbg_backup_ext_img_cmd,
        AIC_NULL
    },
    {
        "AT^RSEXT",
        AIC_NULL,
        AIC_NULL,
        atdbg_restore_ext_img_cmd,
        AIC_NULL
    },
#endif

    {
        AIC_NULL
    },
};

int aic_sys_debug_at_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)debug_at_handles);

    return AIC_SUCCESS;
}
AIC_APP_P2_INIT_EXPORT(aic_sys_debug_at_init);

