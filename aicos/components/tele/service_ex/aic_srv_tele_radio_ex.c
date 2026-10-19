/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_srv_tele_radio_ex.c
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a29ef
#define LOG_MODULE    AMOD_SYS

/*********************
 *      INCLUDES
 *********************/
#include <aic_osa.h>
#include <aic_math.h>
#include <aic_convert.h>
#include "aic_at_process_common.h"
#include "aic_at_resp_handle.h"
#include "aic_srv_tele.h"
#include "aic_srv_tele_radio.h"
#include "diag_cmd_api.h"
#include "aic_at_interact_cp.h"

/*********************
 *      DEFINES
 *********************/
#ifndef MAX_CMD_LENGTH
#define MAX_CMD_LENGTH (100)
#endif
#define IMEI_STRING_LEN         (15)
#define IMEI_DIAG_READ_SUBCMD   (0)
#define IMEI_DIAG_WRITE_SUBCMD  (1)
#define IMEI_WAIT_TIMEOUT       (3)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/* TODO: delete until imei has correct api */
extern aos_semaphore_ptr s_imei_semp_ptr;
extern ulong s_imei_line_pointers[2];
extern u8 s_imei_line_cnt;

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int32_t aic_srv_tele_radio_get_signal_info_sync(uint8_t card_id, ts_radio_signal_info_t *p_signal_info)
{
    AT_RESP_FROM_CP at_resp = {0};
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16  paras_count = 0;
    int32_t rsrp = 0;

    /* Judge param valid */
    /* No need to judge card id based on TELE_SERVICE_EX */
    if (NULL == p_signal_info) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc response buffer */
    at_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    at_resp.buf_len = AT_MAX_AP_RESP_LENGTH;
    if (NULL == at_resp.resp_buf) {
        aicx_error(0x049ca2, "sd", "[%s] aos_zalloc fail, size is (%d).\r\n", __func__, at_resp.buf_len);
        return -1;
    }

    /* send at commond */
    aic_send_cmd_and_wait("AT+CSQ", &at_resp);

    /* parse at response info. */
    if(strstr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR) || strstr(at_resp.resp_buf, AT_SIMPLE_RESULT_ERROR)) {
        aicx_error(0x0536cd, "s", "[%s] get signal info fail!\r\n", __func__);
        if(NULL != at_resp.resp_buf) {
            aos_free(at_resp.resp_buf);
            at_resp.resp_buf = NULL;
        }
        return -1;
    }

    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
    if (!line_arr) {
        aicx_error(0x12ba55, "s", "[%s]line_arr is null.\r\n", __func__);

        if(NULL != at_resp.resp_buf) {
            aos_free(at_resp.resp_buf);
            at_resp.resp_buf = NULL;
        }
        return -1;
    }

    /* +CSQ: <200+rsrp>,99 */
    para_arr = aic_split_paras_to_str_array((u8 *)line_arr[0],
                                            "+CSQ: ",
                                            &paras_count);

    aicx_info(0x1babf7, "sds", "[%s]paras_count = %d,line_arr[0] = %s\r\n", __func__,paras_count,(char *)line_arr[0]);

    if (paras_count >= 1) {
        /* parse rsrp */
        rsrp = (int32_t)atoi((u8 *)para_arr[0]);
        aicx_info(0x045f7e, "ssd", "[%s]para_arr[0] = %s, rsrp = %d \r\n", __func__,(char *)para_arr[0] ,rsrp);
    }

    p_signal_info->signal_level_val = rsrp;

    /* rsrp: 0:<-140dBm */
    /* rsrp: 1:-140dBm~-139dBm */
    /* rsrp: 2:-139dBm~-138dBm */
    /* rsrp: ... */
    /* rsrp: 95:-46dBm~-45dBm */
    /* rsrp: 96:-45dBm~-44dBm */
    /* rsrp: 97:>=-44dBm */
    if (0 == rsrp) {
        p_signal_info->signal_strength = -150;
    } else if ((0 < rsrp) && (97 > rsrp)) {
        p_signal_info->signal_strength = -140 + rsrp - 1;
    } else if (97 == rsrp) {
        p_signal_info->signal_strength = -44;
    } else {
        p_signal_info->signal_strength = 0;
    }

    /* LTE: 0:<=-120dBm, 1:-111~-119dBm, 2:-106~-110dBm, 3:-91~-105dBm, 4:>=-90dBm */
    if (-120 >= p_signal_info->signal_strength) {
        p_signal_info->signal_level = 0;
    } else if ((-119 <= p_signal_info->signal_strength) && (-111 >= p_signal_info->signal_strength)) {
        p_signal_info->signal_level = 1;
    } else if ((-110 <= p_signal_info->signal_strength) && (-106 >= p_signal_info->signal_strength)) {
        p_signal_info->signal_level = 2;
    } else if ((-105 <= p_signal_info->signal_strength) && (-91 >= p_signal_info->signal_strength)) {
        p_signal_info->signal_level = 3;
    } else {
        p_signal_info->signal_level = 4;
    }

    /* current only support LTE */
    p_signal_info->signal_type = TS_RADIO_NETWORK_TYPE_LTE;

    aicx_info(0x0fa471, "sddd", "[%s]signal_strength = %d, signal_level = %d, signal_level_val = %d.\r\n",
              __func__,
              p_signal_info->signal_strength,
              p_signal_info->signal_level,
              p_signal_info->signal_level_val);

    aic_free_paras_mem(para_arr, paras_count);
    aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);

    if(NULL != at_resp.resp_buf) {
        aos_free(at_resp.resp_buf);
        at_resp.resp_buf = NULL;
    }

    return 0;
}

int32_t aic_srv_tele_radio_get_operator_info_sync(uint8_t card_id, ts_radio_operator_info_t *p_operator_info)
{
    char cmd[MAX_CMD_LENGTH] = {0};
    AT_RESP_FROM_CP at_resp = {0};
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16 paras_count = 0, i = 0;
    int ret = 0;

    /* No need to judge card id based on TELE_SERVICE_EX */
    if (NULL == p_operator_info) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc response buffer */
    at_resp.buf_len  = AT_MAX_AP_RESP_LENGTH;
    at_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    if (NULL == at_resp.resp_buf) {
        aicx_error(0x049ca2, "sd", "[%s] aos_zalloc fail, size is (%d).\r\n", __func__, at_resp.buf_len);
        return -1;
    }

    for (i = 0; i < 3; i++) {/* AT+COPS=3,0;AT+COPS?;AT+COPS=3,1;AT+COPS?;AT+COPS=3,2;AT+COPS? */

        snprintf(cmd, MAX_CMD_LENGTH, "AT+COPS=3,%d", i);
        /* send AT commond to get long_name */
        aic_send_cmd_and_wait(cmd, AIC_NULL);

        /* memset at_resp.resp_buf first */
        aos_memset(at_resp.resp_buf, 0, AT_MAX_AP_RESP_LENGTH);

        aic_send_cmd_and_wait("AT+COPS?", &at_resp);

        /* parse AT response data */
        if(strcasestr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR) || strcasestr(at_resp.resp_buf, AT_SIMPLE_RESULT_ERROR)) {
            aicx_error(0x1631c5, "s", "[%s] get operator long name fail.\r\n", __func__);
            aos_free(at_resp.resp_buf);
            return -1;
        }

        /* check if is the right response */
        if(!strcasestr(at_resp.resp_buf, "+COPS:")) {
            aicx_error(0x17a378, "ss", "[%s] get error at_resp %s.\r\n", __func__, at_resp.resp_buf);
            aos_free(at_resp.resp_buf);
            return -1;
        }

        line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
        if (!line_arr) {
            aicx_error(0x12ba55, "s", "[%s]line_arr is null.\r\n", __func__);
            aos_free(at_resp.resp_buf);
            return -1;
        }
        aicx_info(0x0d33d7, "ss", "[%s] get operator: %s\r\n", __func__, (char *)at_resp.resp_buf);

        para_arr = aic_split_paras_to_str_array((u8 *)line_arr[0], "+COPS: ", &paras_count);

        if (paras_count >= 3) {
            int32_t format = 0;
            aicx_info(0x1daa0e, "sss", "[%s]para_arr[1] = %s para_arr[2] = %s \r\n", __func__, (char *)para_arr[1], (char *)para_arr[2]);
            format = (int32_t)atoi((u8 *)para_arr[1]);
            if (0 == format) {/* 0: long name */
                aos_memcpy(&(p_operator_info->operator_long_name) , (void *)para_arr[2], strlen((char *)para_arr[2]));
            }
            else if (1 == format) {/* 1: short name */
                aos_memcpy(&(p_operator_info->operator_short_name) , (void *)para_arr[2], strlen((char *)para_arr[2]));
            }
            else if (2 == format) {/* 2: numeric */
                char plmn[TS_RADIO_PLMN_MCC_LEN_MAX * 2 + 1] = {0};
                aos_memcpy(&plmn , (void *)para_arr[2], strlen((char *)para_arr[2]));
                aos_memcpy(&(p_operator_info->plmn_mcc), &plmn ,TS_RADIO_PLMN_MCC_LEN_MAX);
                aos_memcpy(&(p_operator_info->plmn_mnc), &(plmn[TS_RADIO_PLMN_MCC_LEN_MAX]), strlen((char *)para_arr[2]) - TS_RADIO_PLMN_MCC_LEN_MAX);
            }
        }
        else {
            aicx_error(0x1059d5, "sd", "[%s] parser params error, the count(%d) is less than 2.\r\n", __func__, paras_count);
            ret = -1;
        }

        aic_free_paras_mem(para_arr, paras_count);
        aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);
    }

    if (0 == strncmp(p_operator_info->operator_short_name, "CMCC", 4)) {
        p_operator_info->operator_id = TS_RADIO_OPERATOR_ID_CMCC;
    }
    else if (0 == strncmp(p_operator_info->operator_short_name, "UNICOM", 6)) {
        p_operator_info->operator_id = TS_RADIO_OPERATOR_ID_CUCC;
    }
    else if (0 == strncmp(p_operator_info->operator_short_name, "CTC", 3)) {
        p_operator_info->operator_id = TS_RADIO_OPERATOR_ID_CTCC;
    } else {
        p_operator_info->operator_id = TS_RADIO_OPERATOR_ID_UNKNOWN;
    }

    aicx_info(0x05a63c, "sssssd", "[%s]operator_long_name = %s operator_short_name = %s, plmn_mcc = %s, plmn_mnc = %s operator_id = %d \r\n", __func__,
                            p_operator_info->operator_long_name,
                            p_operator_info->operator_short_name,
                            p_operator_info->plmn_mcc,
                            p_operator_info->plmn_mnc,
                            p_operator_info->operator_id);

    aos_free(at_resp.resp_buf);
    return ret;
}

int32_t aic_srv_tele_radio_get_reg_status_sync(uint8_t card_id, ts_radio_reg_status_e *p_reg_status)
{
    AT_RESP_FROM_CP at_resp = {0};
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16  paras_count = 0;

    /* No need to judge card id based on TELE_SERVICE_EX */
    if (NULL == p_reg_status) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc response buffer */
    at_resp.buf_len  = AT_MAX_AP_RESP_LENGTH;
    at_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    if (NULL == at_resp.resp_buf) {
        aicx_error(0x049ca2, "sd", "[%s] aos_zalloc fail, size is (%d).\r\n", __func__, at_resp.buf_len);
        return -1;
    }

    /* send AT commond */
    aic_send_cmd_and_wait("AT+CEREG?", &at_resp);

    /* parse AT response data */
    if(strstr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR) || strstr(at_resp.resp_buf, AT_SIMPLE_RESULT_ERROR)) {
        aicx_error(0x03d4e5, "s", "[%s] get reg status fail.\r\n", __func__);
        aos_free(at_resp.resp_buf);
        return -1;
    }

    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
    if (!line_arr) {
        aicx_error(0x12ba55, "s", "[%s]line_arr is null.\r\n", __func__);

        aos_free(at_resp.resp_buf);
        return -1;
    }
    aicx_info(0x05cd05, "ss", "[%s] get reg status: %s\r\n", __func__, (char *)at_resp.resp_buf);

    /*  */
    para_arr = aic_split_paras_to_str_array((u8 *)line_arr[0],
                                            "+CEREG: ",
                                            &paras_count);

    if (paras_count >= 2) {
        aicx_info(0x05419d, "sss", "[%s] para_arr[0] = %s , para_arr[1] = %s \r\n", 
                        __func__, (char *)para_arr[0], (char *)para_arr[1]);

        /* reg_status */
        if ((strstr((char *)para_arr[1], "1")) || (strstr((char *)para_arr[1], "5"))) {
            *p_reg_status = TS_RADIO_REG_STATUS_IN_SERVICE;
        }
        else {
            *p_reg_status = TS_RADIO_REG_STATUS_NO_SERVICE;
        }
    }
    else {
        aicx_error(0x1059d5, "sd", "[%s] parser params error, the count(%d) is less than 2.\r\n", __func__, paras_count);
    }


    aic_free_paras_mem(para_arr, paras_count);
    aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);

    aos_free(at_resp.resp_buf);
    return 0;
}

int32_t aic_srv_tele_radio_set_current_slot_imei_sync(char *p_imei)
{
    AT_RESP_FROM_CP resp_to_save = {0};
    uint8_t line_idx = 0;
    ulong *p_line_arr = NULL;
    u8 cmd_str[MAX_CMD_LENGTH] = {0};

    /* param check */
    /* No need to judge card id based on TELE_SERVICE_EX */
    if (NULL == p_imei) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    resp_to_save.resp_buf = aos_zalloc(MAX_CMD_LENGTH + 1);
    if (NULL == resp_to_save.resp_buf) {
        aicx_error(0x0c8af9, "s", "[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return - 1;
    }
    resp_to_save.buf_len = MAX_CMD_LENGTH;

    snprintf(cmd_str, sizeof(cmd_str), "AT+SIMEI=%s", p_imei);

    aicx_info(0x076e61, "ss", "[%s]cmd_str = %s.\r\n", __func__, cmd_str);

    /* send at command to set imei str */
    aic_send_cmd_and_wait(cmd_str, &resp_to_save);

    if (resp_to_save.resp_line_cnt) {
        p_line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (NULL == p_line_arr) {
            aicx_error(0x0fc495, "s", "[%s]p_line_arr is NULL.\r\n", __func__);
            aos_free(resp_to_save.resp_buf);
            return -1;
        }
        for (line_idx = 0; line_idx < resp_to_save.resp_line_cnt; line_idx++) {
            /* Contains ERROR string. Maybe it is +CME ERROR: or ERROR. */
            if (strcasestr((u8 *)p_line_arr[line_idx], AT_SIMPLE_RESULT_ERROR)) {
                aicx_error(0x0e4f1e, "s", "[%s]cmd_str return error.\r\n", __func__,cmd_str);
                aos_free(resp_to_save.resp_buf);
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
    }

    aicx_info(0x09eeca, "s", "[%s]exit(return 0).", __func__);

    aos_free(resp_to_save.resp_buf);
    return 0;
}

int32_t aic_srv_tele_radio_get_current_slot_imei_sync(char *p_imei)
{
    AT_RESP_FROM_CP at_resp = {0};
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16  paras_count = 0;

    /* No need to judge card id based on TELE_SERVICE_EX */
    if (NULL == p_imei) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc response buffer */
    at_resp.buf_len  = AT_MAX_AP_RESP_LENGTH;
    at_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    if (NULL == at_resp.resp_buf) {
        aicx_error(0x049ca2, "sd", "[%s] aos_zalloc fail, size is (%d).\r\n", __func__, at_resp.buf_len);
        return -1;
    }

    /* send AT commond */
    aic_send_cmd_and_wait("AT+CGSN=1", &at_resp);

    /* parse AT response data */
    if(strstr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR) || strstr(at_resp.resp_buf, AT_SIMPLE_RESULT_ERROR)) {
        aicx_error(0x049a79, "s", "[%s] get imei fail.\r\n", __func__);
        aos_free(at_resp.resp_buf);
        return -1;
    }

    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
    if (!line_arr) {
        aicx_error(0x12ba55, "s", "[%s]line_arr is null.\r\n", __func__);
        aos_free(at_resp.resp_buf);
        return -1;
    }
    aicx_info(0x1aaae2, "ss", "[%s] get imei: %s\r\n", __func__, (char *)at_resp.resp_buf);

    para_arr = aic_split_paras_to_str_array((u8 *)line_arr[0],
                                            "+CGSN: ",
                                            &paras_count);

    if ((paras_count >= 1) && (AIC_NULL != para_arr)) {
        aicx_info(0x107468, "ss", "[%s] para_arr[0] = %s \r\n", __func__, (char *)para_arr[0]);
        /* get imei string */
        aos_memcpy(p_imei, (void *)para_arr[0], strlen((char *)para_arr[0]));
    }
    else {
        aicx_error(0x05acd7, "sd", "[%s] parser params error, the count(%d) is less than 1.\r\n", __func__, paras_count);
    }

    aic_free_paras_mem(para_arr, paras_count);
    aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);

    aos_free(at_resp.resp_buf);
    return 0;
}

int32_t aic_srv_tele_radio_get_cell_info_sync(uint8_t card_id, ts_radio_cell_info_t *p_cell_info)
{
    AT_RESP_FROM_CP at_resp = {0};
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16  paras_count = 0;

    /* No need to judge card id based on TELE_SERVICE_EX */
    if (NULL == p_cell_info) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc response buffer */
    at_resp.buf_len  = AT_MAX_AP_RESP_LENGTH;
    at_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    if (NULL == at_resp.resp_buf) {
        aicx_error(0x049ca2, "sd", "[%s] aos_zalloc fail, size is (%d).\r\n", __func__, at_resp.buf_len);
        return -1;
    }

    /* memset p_ncell_info_list first */
    aos_memset(p_cell_info, 0, sizeof(ts_radio_cell_info_t));

    /* send AT commond */
    aic_send_cmd_and_wait("AT^AQCI=0", &at_resp);
    /* parse AT response data */
    if(strstr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR) || strstr(at_resp.resp_buf, AT_SIMPLE_RESULT_ERROR)) {
        aicx_error(0x148b07, "s", "[%s] get servingcell fail.\r\n", __func__);
        aos_free(at_resp.resp_buf);
        return -1;
    }

    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
    if (!line_arr) {
        aicx_error(0x12ba55, "s", "[%s]line_arr is null.\r\n", __func__);
        aos_free(at_resp.resp_buf);
        return -1;
    }
    aicx_info(0x07e617, "ss", "[%s] get servingcell: %s\r\n", __func__, (char *)at_resp.resp_buf);

    /* parse cell info */
    para_arr = aic_split_paras_to_str_array((u8 *)line_arr[0],
                                            "^AQCI: ",
                                            &paras_count);
    /* ^AQCI: “scell","rat",roaming_flag,cur_plmn_mcc,cur_plmn_mnc,tac,cellid,
                band,ul_bandwidth,dl_bandwidth,earfcn,pci,rsrp,rsrq,sinr,srxlev */
    if (paras_count >= 18) {
        if (!strcmp((const char *)para_arr[1], "LTE")) {
            p_cell_info->rat_type = TS_RADIO_NETWORK_TYPE_LTE;
        }
        else {/* currently only support LTE */
            p_cell_info->rat_type = TS_RADIO_NETWORK_TYPE_UNKNOWN;
        }

        p_cell_info->is_roaming = (int32_t)atoi((u8 *)para_arr[3]);

        aos_memcpy(p_cell_info->plmn_mcc, (void *)para_arr[4], strlen((char *)para_arr[4]));

        aos_memcpy(p_cell_info->plmn_mnc, (void *)para_arr[5], strlen((char *)para_arr[5]));

        p_cell_info->tac = (int32_t)atoi((u8 *)para_arr[6]);

        p_cell_info->cellid = (int32_t)atoi((u8 *)para_arr[7]);

        p_cell_info->band = (int32_t)atoi((u8 *)para_arr[8]);

        p_cell_info->ul_bandwidth = (int32_t)atoi((u8 *)para_arr[9]);

        p_cell_info->dl_bandwidth = (int32_t)atoi((u8 *)para_arr[10]);

        p_cell_info->earfcn = (int32_t)atoi((u8 *)para_arr[11]);

        p_cell_info->pcid = (int32_t)atoi((u8 *)para_arr[12]);

        p_cell_info->rsrp = (int32_t)atoi((u8 *)para_arr[13]);

        p_cell_info->rsrq = (int32_t)atoi((u8 *)para_arr[14]);

        p_cell_info->sinr = (int32_t)atoi((u8 *)para_arr[15]);

        p_cell_info->srxlev = (int32_t)atoi((u8 *)para_arr[16]);

        p_cell_info->rssi = (int32_t)atoi((u8 *)para_arr[17]);

    }
    else {
        aicx_error(0x1be5f5, "sd", "[%s] parser params error, the count is less than 15.  paras_count = %d \r\n", __func__, paras_count);
    }

    aicx_info(0x1a2888, "sdssddddddddddddd", "[%s]rat_type = %d, mcc = %s, mnc = %s, cellid = %d, pcid = %d, earfcn = %d, tac = %d, rsrp = %d, rsrq = %d, is_roaming = %d, band = %d, ul_bandwidth = %d, dl_bandwidth = %d, sinr = %d, srxlev = %d, rssi = %d \r\n", __func__,
                                        p_cell_info->rat_type,
                                        p_cell_info->plmn_mcc,
                                        p_cell_info->plmn_mnc,
                                        p_cell_info->cellid,
                                        p_cell_info->pcid,
                                        p_cell_info->earfcn,
                                        p_cell_info->tac,
                                        p_cell_info->rsrp,
                                        p_cell_info->rsrq,
                                        p_cell_info->is_roaming,
                                        p_cell_info->band,
                                        p_cell_info->ul_bandwidth,
                                        p_cell_info->dl_bandwidth,
                                        p_cell_info->sinr,
                                        p_cell_info->srxlev,
                                        p_cell_info->rssi);

    aic_free_paras_mem(para_arr, paras_count);
    aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);
    aos_free(at_resp.resp_buf);
    return 0;
}

int32_t aic_srv_tele_radio_get_neighbor_cell_info_list_sync(uint8_t card_id, ts_radio_neighbor_cell_info_list_t *p_ncell_info_list)
{
    AT_RESP_FROM_CP at_resp = {0};
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16 paras_count = 0;
    u16 i = 0;
    u16 cell_idx = 0;

    /* No need to judge card id based on TELE_SERVICE_EX */
    if (NULL == p_ncell_info_list) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc response buffer */
    at_resp.buf_len  = AT_MAX_ONE_RESP_LENGTH;
    at_resp.resp_buf = aos_zalloc(AT_MAX_ONE_RESP_LENGTH);
    if (NULL == at_resp.resp_buf) {
        aicx_error(0x049ca2, "sd", "[%s] aos_zalloc fail, size is (%d).\r\n", __func__, at_resp.buf_len);
        return -1;
    }

    /* memset p_ncell_info_list first */
    aos_memset(p_ncell_info_list, 0, sizeof(ts_radio_neighbor_cell_info_list_t));

    /* send AT commond */
    aic_send_cmd_and_wait("AT^AQCI=1", &at_resp);
    /* parse AT response data */
    if(strstr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR) || strstr(at_resp.resp_buf, AT_SIMPLE_RESULT_ERROR)) {
        aicx_error(0x01652b, "s", "[%s] get neighbourcell fail.\r\n", __func__);
        aos_free(at_resp.resp_buf);
        return -1;
    }

    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
    if (!line_arr) {
        aicx_error(0x12ba55, "s", "[%s]line_arr is null.\r\n", __func__);
        aos_free(at_resp.resp_buf);
        return -1;
    }
    aicx_info(0x112215, "ss", "[%s] get neighbourcell: %s\r\n", __func__, (char *)at_resp.resp_buf);

    for (i = 0; i < at_resp.resp_line_cnt; i++) {
        /* check single line has "^AQCI:" means valid neighbor cell info */
        /* ^AQCI: “ncell","rat",mcc,mnc,tac,cellid,earfcn,pci,rsrp,rsrq,sinr,srxlev */
        if (strstr((u8 *)line_arr[i], "^AQCI:")) {

            cell_idx = p_ncell_info_list->ncell_num;

            /* parse neighbor cell info */
            para_arr = AIC_NULL;
            para_arr = aic_split_paras_to_str_array((u8 *)line_arr[i], "^AQCI: ", &paras_count);

            aicx_info(0x00dcb7, "dsdd", "== parse line_arr[%d] %s paras_count = %d cell_idx = %d == \r\n", i, (u8 *)line_arr[i], paras_count, cell_idx);

            if (paras_count >= 14) {
                if (!strcmp((const char *)para_arr[1], "LTE")) {
                    p_ncell_info_list->ncell_info[cell_idx].rat_type = TS_RADIO_NETWORK_TYPE_LTE;
                }
                else {/* currently only support LTE */
                    p_ncell_info_list->ncell_info[cell_idx].rat_type = TS_RADIO_NETWORK_TYPE_UNKNOWN;
                }

                aos_memcpy(p_ncell_info_list->ncell_info[cell_idx].plmn_mcc, (void *)para_arr[3], strlen((char *)para_arr[3]));

                aos_memcpy(p_ncell_info_list->ncell_info[cell_idx].plmn_mnc, (void *)para_arr[4], strlen((char *)para_arr[4]));

                p_ncell_info_list->ncell_info[cell_idx].tac = (int32_t)atoi((u8 *)para_arr[5]);

                p_ncell_info_list->ncell_info[cell_idx].cellid = (int32_t)atoi((u8 *)para_arr[6]);

                p_ncell_info_list->ncell_info[cell_idx].earfcn = (int32_t)atoi((u8 *)para_arr[7]);

                p_ncell_info_list->ncell_info[cell_idx].pcid = (int32_t)atoi((u8 *)para_arr[8]);

                p_ncell_info_list->ncell_info[cell_idx].rsrp = (int32_t)atoi((u8 *)para_arr[9]);

                p_ncell_info_list->ncell_info[cell_idx].rsrq = (int32_t)atoi((u8 *)para_arr[10]);

                p_ncell_info_list->ncell_info[cell_idx].sinr = (int32_t)atoi((u8 *)para_arr[11]);

                p_ncell_info_list->ncell_info[cell_idx].srxlev = (int32_t)atoi((u8 *)para_arr[12]);

                p_ncell_info_list->ncell_info[cell_idx].rssi = (int32_t)atoi((u8 *)para_arr[13]);

            }

            aic_free_paras_mem(para_arr, paras_count);

            aicx_info(0x1635ac, "sddssddddddddd", "[%s]cell_idx = %d rat_type = %d, mcc = %s, mnc = %s, cellid = %d, earfcn = %d, pcid = %d, rsrq = %d, rsrp = %d, tac = %d, sinr = %d, srxlev = %d, rssi = %d \r\n",
                                    __func__,cell_idx,
                                    p_ncell_info_list->ncell_info[cell_idx].rat_type,
                                    p_ncell_info_list->ncell_info[cell_idx].plmn_mcc,
                                    p_ncell_info_list->ncell_info[cell_idx].plmn_mnc,
                                    p_ncell_info_list->ncell_info[cell_idx].cellid,
                                    p_ncell_info_list->ncell_info[cell_idx].earfcn,
                                    p_ncell_info_list->ncell_info[cell_idx].pcid,
                                    p_ncell_info_list->ncell_info[cell_idx].rsrq,
                                    p_ncell_info_list->ncell_info[cell_idx].rsrp,
                                    p_ncell_info_list->ncell_info[cell_idx].tac,
                                    p_ncell_info_list->ncell_info[cell_idx].sinr,
                                    p_ncell_info_list->ncell_info[cell_idx].srxlev,
                                    p_ncell_info_list->ncell_info[cell_idx].rssi);

            p_ncell_info_list->ncell_num++;
        }

    }

    aicx_info(0x0dc91c, "sd", "[%s] get neighbourcell success, total number = %d \r\n", __func__, p_ncell_info_list->ncell_num);

    aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);
    aos_free(at_resp.resp_buf);
    return 0;
}

int32_t aic_srv_tele_radio_on_sync(uint8_t card_id, bool raido_on)
{
    AT_RESP_FROM_CP resp_to_save = {0};
    uint8_t line_idx = 0;
    ulong *p_line_arr = NULL;

    /* param check */
    /* No need to judge card id based on TELE_SERVICE_EX */

    resp_to_save.resp_buf = aos_zalloc(MAX_CMD_LENGTH + 1);
    if (NULL == resp_to_save.resp_buf) {
        aicx_error(0x0c8af9, "s", "[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return - 1;
    }
    resp_to_save.buf_len = MAX_CMD_LENGTH;

    aicx_info(0x0a09f6, "sd", "[%s]radio_status = %d.\r\n", __func__, raido_on);
    /* send at command to set cfun status */
    if (true == raido_on) {
        aic_send_cmd_and_wait("AT+CFUN=1", &resp_to_save);
    } else {
        aic_send_cmd_and_wait("AT+CFUN=0", &resp_to_save);
    }
    if (resp_to_save.resp_line_cnt) {
        p_line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (NULL == p_line_arr) {
            aicx_error(0x0fc495, "s", "[%s]p_line_arr is NULL.\r\n", __func__);
            aos_free(resp_to_save.resp_buf);
            return -1;
        }
        for (line_idx = 0; line_idx < resp_to_save.resp_line_cnt; line_idx++) {
            /* Contains ERROR string. Maybe it is +CME ERROR: or ERROR. */
            if (strcasestr((u8 *)p_line_arr[line_idx], AT_SIMPLE_RESULT_ERROR)) {
                aicx_error(0x135f67, "s", "[%s]AT+CFUN= return error.\r\n", __func__);
                aos_free(resp_to_save.resp_buf);
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
    }

    aicx_info(0x09eeca, "s", "[%s]exit(return 0).", __func__);

    aos_free(resp_to_save.resp_buf);
    return 0;
}

bool aic_srv_tele_radio_is_on_sync(uint8_t card_id)
{
    AT_RESP_FROM_CP resp_to_save = {0};
    uint8_t line_idx = 0;
    ulong *p_line_arr = NULL;
    ulong *p_para_arr = NULL;
    u16 paras_count = 0;
    u8 radio_status = 0;
    bool ret_radio_status = false;

    /* param check */
    /* No need to judge card id based on TELE_SERVICE_EX */

    resp_to_save.resp_buf = aos_zalloc(MAX_CMD_LENGTH + 1);
    if (NULL == resp_to_save.resp_buf) {
        aicx_error(0x0c8af9, "s", "[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return false;
    }
    resp_to_save.buf_len = MAX_CMD_LENGTH;

    /* send at command to get cfun status */
    aic_send_cmd_and_wait("AT+CFUN?", &resp_to_save);
    if (resp_to_save.resp_line_cnt) {
        p_line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (NULL == p_line_arr) {
            aicx_error(0x0fc495, "s", "[%s]p_line_arr is NULL.\r\n", __func__);
            aos_free(resp_to_save.resp_buf);
            return false;
        }
        for (line_idx = 0; line_idx < resp_to_save.resp_line_cnt; line_idx++) {
            /* Contains ERROR string. Maybe it is +CME ERROR: or ERROR. */
            if (strcasestr((u8 *)p_line_arr[line_idx], AT_SIMPLE_RESULT_ERROR)) {
                aicx_error(0x007f28, "s", "[%s]AT+CFUN? return error.\r\n", __func__);
                aos_free(resp_to_save.resp_buf);
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return false;
            }

            if (strcasestr((u8 *)p_line_arr[line_idx], "+CFUN:")) {
                p_para_arr = aic_split_paras_to_str_array((u8 *)p_line_arr[line_idx],
                                                    "+CFUN: ",
                                                    &paras_count);
                /* +CFUN: <fun> */
                if (p_para_arr && paras_count >= 1) {
                    radio_status = atoi((u8 *)p_para_arr[0]);
                    aicx_info(0x0a09f6, "sd", "[%s]radio_status = %d.\r\n", __func__, radio_status);
                    if (1 == radio_status) {
                        ret_radio_status = true;
                    } else {
                        ret_radio_status = false;
                    }
                }
                aic_free_paras_mem(p_para_arr, paras_count);
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
    }
    aicx_info(0x07d2f0, "sd", "[%s]ret_radio_status = %d.\r\n", __func__, ret_radio_status);

    aos_free(resp_to_save.resp_buf);
    return ret_radio_status;
}
