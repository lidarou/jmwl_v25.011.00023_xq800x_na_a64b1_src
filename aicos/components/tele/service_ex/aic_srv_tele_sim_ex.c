/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_srv_tele_sim_ex.c
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16ddac
#define LOG_MODULE    AMOD_SYS

/*********************
 *      INCLUDES
 *********************/
#ifndef MAX_CMD_LENGTH
#define MAX_CMD_LENGTH (100)
#endif
#include <aic_osa.h>

#include "aic_at_process_common.h"
#include "aic_at_resp_handle.h"
#include "aic_srv_tele.h"
#include "tele_cfg.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int32_t aic_srv_tele_sim_get_status_sync(uint8_t card_id, ts_sim_status_e *p_sim_status)
{
    AT_RESP_FROM_CP at_cpin_resp = {0};
    char             *p_dest_str = NULL;
    ts_sim_status_e   sim_status = TS_SIM_STATUS_NOT_PRESENT;

    /* No need to judge card id based on TELE_SERVICE_EX */

    /* check param valid */
    if (NULL == p_sim_status) {
        aicx_error(0x101d20, "s", "[%s]p_sim_status param error.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc */
    at_cpin_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == at_cpin_resp.resp_buf) {
        aicx_error(0x0e3ec0, "s", "[%s] malloc failed!\r\n", __func__);
        return -1;
    }
    at_cpin_resp.buf_len = AT_MAX_AP_RESP_LENGTH;
    /* send AT command */
    aic_send_cmd_and_wait("at+cpin?", &at_cpin_resp);

    /* judge network is error */
    if(strstr(at_cpin_resp.resp_buf, AT_VERBOSE_RESULT_ERROR)) {
        sim_status = TS_SIM_STATUS_NOT_PRESENT;
        *p_sim_status = sim_status;
        aicx_info(0x13078f, "s", "[%s]*p_sim_status = TS_SIM_STATUS_NOT_PRESENT.\r\n", __func__);
        if(NULL != at_cpin_resp.resp_buf) {
            aos_free(at_cpin_resp.resp_buf);
            at_cpin_resp.resp_buf = NULL;
        }
        return 0;
    }
    /*find string "+CPIN:" */
    p_dest_str = strcasestr(at_cpin_resp.resp_buf, "+CPIN:");
    if (NULL == p_dest_str) {
        aicx_error(0x035a20, "s", "[%s] parse param error.\r\n", __func__);
    } else {
        aicx_info(0x05ba6b, "ss", "[%s] p_dest_str = %s\r\n", __func__,p_dest_str);
        if (strstr(p_dest_str, "READY")) {
            sim_status = TS_SIM_STATUS_READY;
        } else if (strstr(p_dest_str, "SIM PIN")) {
            sim_status = TS_SIM_STATUS_PIN;
        } else if (strstr(p_dest_str, "SIM PUK")) {
            sim_status = TS_SIM_STATUS_PUK;
        } else {
            sim_status = TS_SIM_STATUS_NOT_READY;
        }
    }

    *p_sim_status = sim_status;
    aicx_info(0x19885f, "sd", "[%s]*p_sim_status = %d.\r\n", __func__, *p_sim_status);

    if(NULL != at_cpin_resp.resp_buf) {
        aos_free(at_cpin_resp.resp_buf);
        at_cpin_resp.resp_buf = NULL;
    }

    return 0;
}

int32_t aic_srv_tele_sim_get_imsi_sync(uint8_t card_id, char *p_sim_imsi)
{
    AT_RESP_FROM_CP at_cimi_resp = {0};
    ulong              *line_arr = NULL;

    /* No need to judge card id based on TELE_SERVICE_EX */

    /* check param */
    if (NULL == p_sim_imsi) {
        aicx_error(0x1c2427, "s", "[%s]p_sim_imsi param error, is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc */
    at_cimi_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == at_cimi_resp.resp_buf) {
        aicx_error(0x0e3ec0, "s", "[%s] malloc failed!\r\n", __func__);
        return -1;
    }
    at_cimi_resp.buf_len = AT_MAX_AP_RESP_LENGTH;
    /* send AT+CIMI command,respnse: */
    aic_send_cmd_and_wait("at+cimi", &at_cimi_resp);

    /* judge network is error */
    if(strstr(at_cimi_resp.resp_buf, AT_VERBOSE_RESULT_ERROR)) {
        aicx_error(0x1dc049, "s", "[%s] get imsi fail!\r\n", __func__);
        if(NULL != at_cimi_resp.resp_buf) {
            aos_free(at_cimi_resp.resp_buf);
            at_cimi_resp.resp_buf = NULL;
        }
        return -1;
    }
    /* resp split string as line */
    line_arr = aic_get_each_line_from_resp(at_cimi_resp.resp_buf, at_cimi_resp.resp_line_cnt);

    aicx_info(0x15d359, "sss", "[%s]at_cimi_resp.resp_buf = %s,line_arr[0] = %s.\r\n", __func__,at_cimi_resp.resp_buf,(char *)line_arr[0]);
    /*imsi max length is 15 */
    strncpy(p_sim_imsi,(char *)line_arr[0],strlen((char *)line_arr[0]));
    aicx_info(0x0bf447, "ss", "[%s]p_sim_imsi = %s.\r\n", __func__,p_sim_imsi);

    /* free params */
    aic_free_paras_mem(line_arr, at_cimi_resp.resp_line_cnt);
    if(NULL != at_cimi_resp.resp_buf) {
        aos_free(at_cimi_resp.resp_buf);
        at_cimi_resp.resp_buf = NULL;
    }

    return 0;
}

int32_t aic_srv_tele_sim_get_plmn_sync(uint8_t card_id, char *p_plmn_mcc,char *p_plmn_mnc)
{
    AT_RESP_FROM_CP at_cimi_resp = {0};
    ulong              *line_arr = NULL;

    /* No need to judge card id based on TELE_SERVICE_EX */

    /* check param */
    if (NULL == p_plmn_mcc || NULL == p_plmn_mnc) {
        aicx_error(0x07fc3e, "s", "[%s]p_plmn_mcc or p_plmn_mnc is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc */
    at_cimi_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == at_cimi_resp.resp_buf) {
        aicx_error(0x0e3ec0, "s", "[%s] malloc failed!\r\n", __func__);
        return -1;
    }
    at_cimi_resp.buf_len = AT_MAX_AP_RESP_LENGTH;
    /*send AT command AT+CIMI */
    aic_send_cmd_and_wait("at+cimi", &at_cimi_resp);

    /* judge network is error */
    if(strstr(at_cimi_resp.resp_buf, AT_VERBOSE_RESULT_ERROR)) {
        aicx_error(0x033862, "s", "[%s] get plmn fail!\r\n", __func__);
        if(NULL != at_cimi_resp.resp_buf) {
            aos_free(at_cimi_resp.resp_buf);
            at_cimi_resp.resp_buf = NULL;
        }
        return -1;
    }

    /* resp split string as line */
    line_arr = aic_get_each_line_from_resp(at_cimi_resp.resp_buf, at_cimi_resp.resp_line_cnt);
    aicx_info(0x15d359, "sss", "[%s]at_cimi_resp.resp_buf = %s,line_arr[0] = %s.\r\n", __func__,at_cimi_resp.resp_buf,(char *)line_arr[0]);
    /*first 3 digit num is mcc;then 2 digit num is mnc */
    strncpy(p_plmn_mcc,(char *)line_arr[0],3);
    /* TODO: mnc maybe 3 digits, and currently copy 2 digits. */
    strncpy(p_plmn_mnc,(char *)(line_arr[0]+3),2);
    aicx_info(0x094ff4, "sss", "[%s]p_plmn_mcc = %s,p_plmn_mnc = %s.\r\n", __func__,p_plmn_mcc,p_plmn_mnc);

    /* free params */
    aic_free_paras_mem(line_arr, at_cimi_resp.resp_line_cnt);
    if(NULL != at_cimi_resp.resp_buf) {
        aos_free(at_cimi_resp.resp_buf);
        at_cimi_resp.resp_buf = NULL;
    }

    return 0;
}

int32_t aic_srv_tele_sim_get_iccid_sync(uint8_t card_id, char *p_sim_iccid)
{
    AT_RESP_FROM_CP at_resp = {0};
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16  paras_count = 0;

    /* No need to judge card id based on TELE_SERVICE_EX */

    /* check param */
    if (NULL == p_sim_iccid) {
        aicx_error(0x123a5d, "s", "[%s]p_sim_iccid error is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc */
    at_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == at_resp.resp_buf) {
        aicx_error(0x0e3ec0, "s", "[%s] malloc failed!\r\n", __func__);
        return -1;
    }
    at_resp.buf_len = AT_MAX_AP_RESP_LENGTH;

    /* send AT command AT^DCID */
    aic_send_cmd_and_wait("AT^DCID", &at_resp);

    /* judge network is error */
    if(strstr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR) || strstr(at_resp.resp_buf, AT_SIMPLE_RESULT_ERROR)) {
        aicx_error(0x05a529, "s", "[%s] get iccid fail!\r\n", __func__);
        if(NULL != at_resp.resp_buf) {
            aos_free(at_resp.resp_buf);
            at_resp.resp_buf = NULL;
        }
        return -1;
    }

    /* resp split string as line */
    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);

    if (!line_arr) {
        aicx_error(0x12ba55, "s", "[%s]line_arr is null.\r\n", __func__);
        aos_free(at_resp.resp_buf);
        at_resp.resp_buf = NULL;
        return -1;
    }

    aicx_info(0x1477d7, "sss", "[%s]at_resp.resp_buf = %s,line_arr[0] = %s.\r\n", __func__,at_resp.resp_buf,(char *)line_arr[0]);
    aicx_info(0x1d36c9, "ss", "[%s] get iccid: %s\r\n", __func__, (char *)at_resp.resp_buf);

    /*parse iccid */
    para_arr = aic_split_paras_to_str_array((u8 *)line_arr[0],
                                            "^DCID: ",
                                            &paras_count);

    if ((paras_count >= 1) && (AIC_NULL != para_arr)) {
        aicx_info(0x107468, "ss", "[%s] para_arr[0] = %s \r\n", __func__, (char *)para_arr[0]);
        /* get iccid string */
        aos_memcpy(p_sim_iccid, (void *)para_arr[0], strlen((char *)para_arr[0]));
    }
    else {
        aicx_error(0x05acd7, "sd", "[%s] parser params error, the count(%d) is less than 1.\r\n", __func__, paras_count);
    }

    aic_free_paras_mem(para_arr, paras_count);
    aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);

    aos_free(at_resp.resp_buf);

    return 0;
}

int32_t aic_srv_tele_sim_get_msisdn_sync(uint8_t card_id, char *p_sim_msisdn)
{
    AT_RESP_FROM_CP at_cnum_resp = {0};
    ulong              *line_arr = NULL;
    ulong             *param_arr = NULL;
    u16                param_cnt = 0;

    /* No need to judge card id based on TELE_SERVICE_EX */

    /* check param */
    if (NULL == p_sim_msisdn) {
        aicx_error(0x0dc37a, "s", "[%s]p_sim_msisdn param error is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* malloc */
    at_cnum_resp.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == at_cnum_resp.resp_buf) {
        aicx_error(0x0e3ec0, "s", "[%s] malloc failed!\r\n", __func__);
        return -1;
    }
    at_cnum_resp.buf_len = AT_MAX_AP_RESP_LENGTH;
    /* send at command */
    aic_send_cmd_and_wait("at+cnum",&at_cnum_resp);

    aicx_info(0x0e8403, "ss", "[%s]at_cnum_resp.resp_buf = %s.\r\n", __func__,at_cnum_resp.resp_buf);

    /* judge return value error */
    if(strstr(at_cnum_resp.resp_buf, AT_VERBOSE_RESULT_ERROR)) {
        aicx_error(0x1abd4e, "s", "[%s] get msisdn fail!\r\n", __func__);
        if(NULL != at_cnum_resp.resp_buf) {
            aos_free(at_cnum_resp.resp_buf);
            at_cnum_resp.resp_buf = NULL;
        }
        return -1;
    }

    line_arr = aic_get_each_line_from_resp(at_cnum_resp.resp_buf, at_cnum_resp.resp_line_cnt);

    if(NULL != (u8 *)line_arr[0]) {
        param_arr = aic_split_paras_to_str_array((u8 *)line_arr[0], "+CNUM:", &param_cnt);
        aicx_info(0x1274be, "ssd", "[%s]param_arr[0] = %s,param_cnt = %d.\r\n", __func__,(char *)param_arr[1],param_cnt);
        if(NULL !=(char *) param_arr[1]) {
            if(strstr((char *)param_arr[1],"+")) {
                /* delete string +86*/
                strncpy(p_sim_msisdn,(char *)(param_arr[1]+3),(strlen((char *)param_arr[1])-3));
            } else {
                strncpy(p_sim_msisdn,(char *)param_arr[1],strlen((char *)param_arr[1]));
            }

            aicx_info(0x07443a, "ss", "[%s],p_sim_msisdn = %s",__func__,p_sim_msisdn);
            /*free param_arr*/
            aic_free_paras_mem(param_arr, param_cnt);
        }else {
            aicx_error(0x022f20, "s", "[%s],param_arr is null",__func__);
        }
        /* free line_arr */
        aic_free_paras_mem(line_arr, at_cnum_resp.resp_line_cnt);
    } else {
        aicx_error(0x1712d3, "s", "[%s],line_arr is null",__func__);
    }

    /* free at_cnum_resp.resp_buf */
    if(NULL != at_cnum_resp.resp_buf) {
        aos_free(at_cnum_resp.resp_buf);
        at_cnum_resp.resp_buf = NULL;
    }

    return 0;
}

int32_t aic_srv_tele_sim_select_card_sync(uint8_t card_id)
{
    AT_RESP_FROM_CP resp_to_save = {0};
    uint8_t line_idx = 0;
    uint8_t slot_cnt = 0;

    ulong *p_line_arr = NULL;
    u8 cmd_str[MAX_CMD_LENGTH + 1] = {0};

    /* check card_id valid */
    slot_cnt = tele_cfg_get_sim_slot_count();
    if (1 == slot_cnt) {
        alog_error("[%s] single card, do not support sim select.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    if (card_id >= TS_CARD_ID_MAX) {
        alog_error("[%s] card_id (%d) error.\r\n", __func__, card_id);
        return TS_ERR_PARAMETER_INVALID;
    }

    resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == resp_to_save.resp_buf) {
        alog_error("[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return TS_ERR_NO_MEMORY;
    }
    resp_to_save.buf_len = MAX_CMD_LENGTH;

    /* 1. switch sim slot by AT^SIMSLOT=<slot_id> */
    snprintf(cmd_str, sizeof(cmd_str), "AT^SIMSLOT=%d", card_id);
    alog_info("[%s]card_id = %d, cmd_str is %s.\r\n", __func__, card_id, cmd_str);

    aic_send_cmd_and_wait(cmd_str, &resp_to_save);
    /* check response result */
    if (resp_to_save.resp_line_cnt) {
        p_line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (NULL == p_line_arr) {
            alog_error("[%s]p_line_arr is NULL.\r\n", __func__);
            if(NULL != resp_to_save.resp_buf) {
                aos_free(resp_to_save.resp_buf);
                resp_to_save.resp_buf = NULL;
            }
            return TS_ERR_PARSE_FAIL;
        }
        for (line_idx = 0; line_idx < resp_to_save.resp_line_cnt; line_idx++) {
            /* Contains ERROR string. Maybe it is +CME ERROR: or ERROR. */
            if (strcasestr((u8 *)p_line_arr[line_idx], AT_SIMPLE_RESULT_ERROR)) {
                alog_error("[%s]cmd_str %s return error.\r\n", __func__, cmd_str);
                if(NULL != resp_to_save.resp_buf) {
                    aos_free(resp_to_save.resp_buf);
                    resp_to_save.resp_buf = NULL;
                }
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
        p_line_arr = NULL;
    }

    /* 2. switch slot to slot_id by gpio if SIM_SLOT_SWITCHED_BY_GPIO enabled */
    tele_cfg_switch_sim_slot_by_gpio(card_id);

    /* 3. active sim slot by AT^AEM=1,"ACT" */
    aos_memset(resp_to_save.resp_buf, 0, resp_to_save.buf_len);

    aic_send_cmd_and_wait("AT^AEM=1,\"ACT\"", &resp_to_save);
    /* check response result */
    if (resp_to_save.resp_line_cnt) {
        p_line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (NULL == p_line_arr) {
            alog_error("[%s]p_line_arr is NULL.\r\n", __func__);
            if(NULL != resp_to_save.resp_buf) {
                aos_free(resp_to_save.resp_buf);
                resp_to_save.resp_buf = NULL;
            }
            return TS_ERR_PARSE_FAIL;
        }
        for (line_idx = 0; line_idx < resp_to_save.resp_line_cnt; line_idx++) {
            /* Contains ERROR string. Maybe it is +CME ERROR: or ERROR. */
            if (strcasestr((u8 *)p_line_arr[line_idx], AT_SIMPLE_RESULT_ERROR)) {
                alog_error("[%s] AT^AEM=1,\"ACT\" return error.\r\n", __func__);
                if(NULL != resp_to_save.resp_buf) {
                    aos_free(resp_to_save.resp_buf);
                    resp_to_save.resp_buf = NULL;
                }
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
        p_line_arr = NULL;
    }

    if(NULL != resp_to_save.resp_buf) {
        aos_free(resp_to_save.resp_buf);
        resp_to_save.resp_buf = NULL;
    }

    return 0;
}

