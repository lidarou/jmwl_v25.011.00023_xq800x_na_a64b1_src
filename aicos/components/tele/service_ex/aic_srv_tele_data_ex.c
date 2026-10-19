/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_srv_tele_data_ex.c
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x098a4c
#define LOG_MODULE    AMOD_SYS

/*********************
 *      INCLUDES
 *********************/
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include "aic_srv_tele.h"
#include "aic_at_process_common.h"
#include "aic_at_resp_handle.h"
#ifdef CONFIG_PDP_MGR
#include "aic_net_pdp_mgr.h"
#endif

/*********************
 *      DEFINES
 *********************/
#define TS_DATA_DEFAULT_PDP_CID         (1)
#define TS_DATA_PDP_CID_VALID_VALUE_MAX (5)

#define TS_DATA_AT_CMD_STR_LEN_MAX      (20)
#define TS_DATA_AT_CMD_RESP_LEN_MAX     (100)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static uint32_t g_ts_data_pdp_id = 0;

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int32_t aic_srv_tele_data_activate_pdp_sync(uint8_t card_id, ts_data_pdp_activate_info_t *p_pdp_act_info, uint32_t *p_pdp_id)
{
#ifdef CONFIG_PDP_MGR
    s32 ret = -1;
#endif

    if (NULL == p_pdp_id) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* param check */
    if (0 != card_id) {
        aicx_error(0x19945f, "s", "[%s]card_id is error.\r\n", __func__);
        *p_pdp_id = 0;
        return -1;
    }

    if (NULL == p_pdp_act_info) {
        aicx_error(0x1bfd4c, "s", "[%s]p_pdp_req_info is NULL.\r\n", __func__);
        *p_pdp_id = 0;
        return -1;
    }

    aicx_info(0x030b45, "sdsssdd", "[%s]apn_type = %d, apn = %s, user_name = %s, password = %s, pdp_type = %d, auth_type = %d.\r\n",
              __func__,
              p_pdp_act_info->apn_type,
              p_pdp_act_info->apn,
              p_pdp_act_info->user_name,
              p_pdp_act_info->password,
              p_pdp_act_info->pdp_type,
              p_pdp_act_info->auth_type);
    g_ts_data_pdp_id = TS_DATA_DEFAULT_PDP_CID;

#ifdef CONFIG_PDP_MGR
    if (TS_DATA_PDP_TYPE_IPV4V6 == p_pdp_act_info->pdp_type) {
        ret = aic_enable_data_connection(g_ts_data_pdp_id, "IPV4V6", p_pdp_act_info->apn);
    } else if (TS_DATA_PDP_TYPE_IPV4 == p_pdp_act_info->pdp_type) {
        ret = aic_enable_data_connection(g_ts_data_pdp_id, "IP", p_pdp_act_info->apn);
    } else if (TS_DATA_PDP_TYPE_IPV6 == p_pdp_act_info->pdp_type) {
        ret = aic_enable_data_connection(g_ts_data_pdp_id, "IPV6", p_pdp_act_info->apn);
    } else {
        ret = aic_enable_data_connection(g_ts_data_pdp_id, "IPV4V6", p_pdp_act_info->apn);
    }
    if (ret) {
        aicx_error(0x1928a1, "sd", "[%s]the ret of called aic_enable_data_connection is %d.\r\n", __func__, ret);
        *p_pdp_id = 0;
        return -1;
    }
#endif

    aicx_info(0x0783cd, "sd", "[%s]the return pdp id is %d.\r\n", __func__, g_ts_data_pdp_id);
    *p_pdp_id = g_ts_data_pdp_id;
    return 0;
}

int32_t aic_srv_tele_data_deactivate_pdp_sync(uint32_t pdp_id)
{
#ifdef CONFIG_PDP_MGR
    s32 ret = -1;
#endif

    aicx_info(0x1997dc, "sd", "[%s]the deactivate pdp id is %d.\r\n", __func__, pdp_id);
#ifdef CONFIG_PDP_MGR
    ret = aic_disable_data_connection(pdp_id);
    if (ret) {
        aicx_error(0x195a44, "sd", "[%s]the ret of called aic_disable_data_connection is %d.\r\n", __func__, ret);
        return -1;
    }
#endif

    aicx_info(0x03be1d, "s", "[%s]deactivate pdp success.\r\n", __func__);
    return 0;
}

int32_t aic_srv_tele_data_get_pdp_status_sync(uint32_t pdp_id, ts_data_pdp_status_e *p_pdp_status)
{
    AT_RESP_FROM_CP resp_to_save = {0};
    uint8_t line_idx = 0;
    ulong *p_line_arr = NULL;
    ulong *p_para_arr = NULL;
    u16 paras_count = 0;
    u8 cid = 0;
    u8 is_pdp_actived = 0;

    /* param check */
    if ((TS_DATA_DEFAULT_PDP_CID > pdp_id) || (TS_DATA_PDP_CID_VALID_VALUE_MAX < pdp_id)) {
        aicx_error(0x1218fb, "sd", "[%s]pdp_id(%d) is invalid.\r\n", __func__, pdp_id);
        return -1;
    }

    if (NULL == p_pdp_status) {
        aicx_error(0x054bcc, "s", "[%s]p_pdp_status is NULL.\r\n", __func__);
        return -1;
    }

    resp_to_save.resp_buf = aos_zalloc(TS_DATA_AT_CMD_RESP_LEN_MAX + 1);
    if (NULL == resp_to_save.resp_buf) {
        aicx_error(0x0c8af9, "s", "[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return - 1;
    }
    resp_to_save.buf_len = TS_DATA_AT_CMD_RESP_LEN_MAX;

    /* send at command to get pdp status */
    aic_send_cmd_and_wait("AT+CGACT?", &resp_to_save);
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
                aicx_error(0x063bb1, "s", "[%s]AT+CGACT? return error.\r\n", __func__);
                aos_free(resp_to_save.resp_buf);
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }

            if (strcasestr((u8 *)p_line_arr[line_idx], "+CGACT:")) {
                p_para_arr = aic_split_paras_to_str_array((u8 *)p_line_arr[line_idx],
                                                    "+CGACT: ",
                                                    &paras_count);
                /* +CGACT: <cid>,<state> */
                if (p_para_arr && paras_count >= 1) {
                    cid = atoi((u8 *)p_para_arr[0]);
                    aicx_info(0x124859, "sd", "[%s]cid = %d.\r\n", __func__, cid);
                    if (cid == pdp_id) {
                        if (paras_count >= 2) {
                            if (p_para_arr[1]) {
                                is_pdp_actived = atoi((u8 *)p_para_arr[1]);
                                aicx_info(0x1c30e3, "sd", "[%s]is_pdp_actived = %d.\r\n", __func__, is_pdp_actived);
                            } else {
                                aicx_info(0x02c14e, "s", "[%s]p_para_arr[1] is NULL.\r\n", __func__);
                            }
                        }
                        aic_free_paras_mem(p_para_arr, paras_count);
                        break;
                    }
                }
                aic_free_paras_mem(p_para_arr, paras_count);
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
    }
    if (0 == is_pdp_actived) {
        *p_pdp_status = TS_DATA_PDP_STATUS_DEACTIVED;
        aicx_info(0x11137e, "s", "[%s]pdp is not actived.\r\n", __func__);
    } else {
        *p_pdp_status = TS_DATA_PDP_STATUS_ACTIVED;
        aicx_info(0x13bea6, "s", "[%s]pdp is actived.\r\n", __func__);
    }

    aos_free(resp_to_save.resp_buf);
    return 0;
}

int32_t aic_srv_tele_data_get_pdp_ip_sync(uint32_t pdp_id, char *p_ip)
{
    AT_RESP_FROM_CP resp_to_save = {0};
    u8 at_cmd_str[TS_DATA_AT_CMD_STR_LEN_MAX + 1] = {0};
    uint8_t line_idx = 0;
    ulong *p_line_arr = NULL;
    ulong *p_para_arr = NULL;
    u16 paras_count = 0;
    u8 cid = 0;
    ts_data_pdp_status_e pdp_status = TS_DATA_PDP_STATUS_MAX;
    int ret = -1;

    /* param check */
    if ((TS_DATA_DEFAULT_PDP_CID > pdp_id) || (TS_DATA_PDP_CID_VALID_VALUE_MAX < pdp_id)) {
        aicx_error(0x1218fb, "sd", "[%s]pdp_id(%d) is invalid.\r\n", __func__, pdp_id);
        return -1;
    }

    if (NULL == p_ip) {
        aicx_error(0x0487e8, "s", "[%s]p_ip is NULL.\r\n", __func__);
        return -1;
    }

    /* if pdp status is deactived, return error. */
    ret = aic_srv_tele_data_get_pdp_status_sync(pdp_id, &pdp_status);
    if (ret) {
        aicx_error(0x037a62, "sd", "[%s]aic_srv_tele_data_get_pdp_status_sync return(%d) error.\r\n", __func__, ret);
        return -1;
    }
    if (TS_DATA_PDP_STATUS_ACTIVED != pdp_status) {
        aicx_error(0x0fe1b5, "s", "[%s]pdp is not activated.\r\n", __func__);
        return -1;
    }

    resp_to_save.resp_buf = aos_zalloc(TS_DATA_AT_CMD_RESP_LEN_MAX + 1);
    if (NULL == resp_to_save.resp_buf) {
        aicx_error(0x0c8af9, "s", "[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return - 1;
    }
    resp_to_save.buf_len = TS_DATA_AT_CMD_RESP_LEN_MAX;

    /* send at command to get ip address */
    sprintf(at_cmd_str, "AT+CGPADDR=%ld", pdp_id);
    aic_send_cmd_and_wait(at_cmd_str, &resp_to_save);

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
                aicx_error(0x1bc4de, "sld", "[%s]AT+CGPADDR=%ld return error.\r\n", __func__, pdp_id);
                aos_free(resp_to_save.resp_buf);
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }

            if (strcasestr((u8 *)p_line_arr[line_idx], "+CGPADDR:")) {
                p_para_arr = aic_split_paras_to_str_array((u8 *)p_line_arr[line_idx],
                                                    "+CGPADDR: ",
                                                    &paras_count);
                /* +CGPADDR: <cid>[,<PDP_ddr_1>[,<PDP_addr_2>]] */
                if (p_para_arr && paras_count >= 1) {
                    cid = atoi((u8 *)p_para_arr[0]);
                    aicx_info(0x124859, "sd", "[%s]cid = %d.\r\n", __func__, cid);
                    if (cid == pdp_id) {
                        if (paras_count >= 2) {
                            if (p_para_arr[1]) {
                                strncpy(p_ip, (u8 *)p_para_arr[1], strlen((u8 *)p_para_arr[1]));
                                aicx_info(0x1c7f1e, "ss", "[%s]p_ip = %s.\r\n", __func__, p_ip);
                            } else {
                                aicx_info(0x02c14e, "s", "[%s]p_para_arr[1] is NULL.\r\n", __func__);
                            }
                        }
                        aic_free_paras_mem(p_para_arr, paras_count);
                        break;
                    }
                }
                aic_free_paras_mem(p_para_arr, paras_count);
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
    }

    aicx_info(0x076d2a, "ss", "[%s]exit.(p_ip = %s)\r\n", __func__, p_ip);
    aos_free(resp_to_save.resp_buf);
    return 0;
}

int32_t aic_srv_tele_data_get_current_pdp_info_sync(ts_data_pdp_info_t *p_pdp_info)
{
    int ret = -1;
    AT_RESP_FROM_CP resp_to_save = {0};
    uint8_t line_idx = 0;
    ulong *p_line_arr = NULL;
    ulong *p_para_arr = NULL;
    u16 paras_count = 0;

    /* param check */
    if (NULL == p_pdp_info) {
        aicx_error(0x19573a, "s", "[%s] pointer is NULL.\r\n", __func__);
        return TS_ERR_PARAMETER_INVALID;
    }

    /* get pdp status */
    ret = aic_srv_tele_data_get_pdp_status_sync(g_ts_data_pdp_id, &(p_pdp_info->pdp_status));
    if (ret) {
        aicx_error(0x14186e, "s", "[%s]aic_srv_tele_data_get_pdp_status_sync fail.\r\n", __func__);
        return -1;
    }

    resp_to_save.resp_buf = aos_zalloc(TS_DATA_AT_CMD_RESP_LEN_MAX + 1);
    if (NULL == resp_to_save.resp_buf) {
        aicx_error(0x0c8af9, "s", "[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return - 1;
    }
    resp_to_save.buf_len = TS_DATA_AT_CMD_RESP_LEN_MAX;

    /* get primary cid */
    /* send at command at+cgcontrdp to get primary cid, bearer id and apn */
    aic_send_cmd_and_wait("AT+CGCONTRDP", &resp_to_save);

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
                aicx_error(0x062d1d, "s", "[%s]AT+CGCONTRDP return error.\r\n", __func__);
                aos_free(resp_to_save.resp_buf);
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }

            if (strcasestr((u8 *)p_line_arr[line_idx], "+CGCONTRDP:")) {
                p_para_arr = aic_split_paras_to_str_array((u8 *)p_line_arr[line_idx],
                                                    "+CGCONTRDP: ",
                                                    &paras_count);
                /* +CGCONTRDP: <cid>,<bearer_id>,<apn> */
                if (p_para_arr && paras_count >= 3) {
                    p_pdp_info->link_info.cid = atoi((u8 *)p_para_arr[0]);
                    strncpy(p_pdp_info->link_info.apn_name, (u8 *)p_para_arr[2], strlen((u8 *)p_para_arr[2]));
                    aic_free_paras_mem(p_para_arr, paras_count);
                    break;
                }
                aic_free_paras_mem(p_para_arr, paras_count);
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
    }

    aicx_info(0x01e6af, "sds", "[%s]exit.(cid = %d, apn = %s)\r\n",
              __func__,
              p_pdp_info->link_info.cid,
              p_pdp_info->link_info.apn_name);
    aos_free(resp_to_save.resp_buf);
    return 0;
}

