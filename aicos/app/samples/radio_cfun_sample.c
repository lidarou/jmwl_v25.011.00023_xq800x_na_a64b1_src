/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file radio_cfun_sample.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_osa_ex.h>
#include <aic_type.h>
#include "aic_at_process_common.h"
#include "aic_at_resp_handle.h"

/*********************
 *      DEFINES
 *********************/
#ifndef MAX_CMD_LENGTH
#define MAX_CMD_LENGTH (100)
#endif

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
int radio_set_cfun_status_sync(uint8_t card_id, uint8_t status)
{
    AT_RESP_FROM_CP resp_to_save = {0};
    uint8_t line_idx = 0;
    ulong *p_line_arr = NULL;
    u8 at_cmd_str[MAX_CMD_LENGTH + 1] = {0};

    /* param check */
    if (0 != card_id) {
        app_loge("[%s] card_id(%d) is invalid.\r\n", __func__, card_id);
        return -1;
    }

    resp_to_save.resp_buf = aic_zalloc(MAX_CMD_LENGTH + 1);
    if (NULL == resp_to_save.resp_buf) {
        app_loge("[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return - 1;
    }
    resp_to_save.buf_len = MAX_CMD_LENGTH;

    app_logi("[%s]status = %d.\r\n", __func__, status);
    /* send at command to set cfun status */
    sprintf(at_cmd_str, "AT+CFUN=%d", status);
    aic_send_cmd_and_wait(at_cmd_str, &resp_to_save);

    if (resp_to_save.resp_line_cnt) {
        p_line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (NULL == p_line_arr) {
            app_loge("[%s]p_line_arr is NULL.\r\n", __func__);
            aic_free(resp_to_save.resp_buf);
            return -1;
        }
        for (line_idx = 0; line_idx < resp_to_save.resp_line_cnt; line_idx++) {
            /* Contains ERROR string. Maybe it is +CME ERROR: or ERROR. */
            if (strcasestr((u8 *)p_line_arr[line_idx], AT_SIMPLE_RESULT_ERROR)) {
                app_loge("[%s]AT+CFUN= return error.\r\n", __func__);
                aic_free(resp_to_save.resp_buf);
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
    }

    app_logi("[%s]exit(return 0).", __func__);

    aic_free(resp_to_save.resp_buf);
    return 0;
}

int radio_get_cfun_status_sync(uint8_t card_id, uint8_t *p_status)
{
    AT_RESP_FROM_CP resp_to_save = {0};
    uint8_t line_idx = 0;
    ulong *p_line_arr = NULL;
    ulong *p_para_arr = NULL;
    u16 paras_count = 0;

    /* param check */
    if (0 != card_id) {
        app_loge("[%s] card_id(%d) is invalid.\r\n", __func__, card_id);
        return -1;
    }

    if (NULL == p_status) {
        app_loge("[%s] p_status is NULL.\r\n", __func__);
        return -1;
    }

    resp_to_save.resp_buf = aic_zalloc(MAX_CMD_LENGTH + 1);
    if (NULL == resp_to_save.resp_buf) {
        app_loge("[%s]resp_to_save.resp_buf malloc fail.\r\n", __func__);
        return - 1;
    }
    resp_to_save.buf_len = MAX_CMD_LENGTH;

    /* send at command to get cfun status */
    aic_send_cmd_and_wait("AT+CFUN?", &resp_to_save);

    if (resp_to_save.resp_line_cnt) {
        p_line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (NULL == p_line_arr) {
            app_loge("[%s]p_line_arr is NULL.\r\n", __func__);
            aic_free(resp_to_save.resp_buf);
            return -1;
        }
        for (line_idx = 0; line_idx < resp_to_save.resp_line_cnt; line_idx++) {
            /* Contains ERROR string. Maybe it is +CME ERROR: or ERROR. */
            if (strcasestr((u8 *)p_line_arr[line_idx], AT_SIMPLE_RESULT_ERROR)) {
                app_loge("[%s]AT+CFUN? return error.\r\n", __func__);
                aic_free(resp_to_save.resp_buf);
                aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
                return -1;
            }

            if (strcasestr((u8 *)p_line_arr[line_idx], "+CFUN:")) {
                p_para_arr = aic_split_paras_to_str_array((u8 *)p_line_arr[line_idx],
                                                    "+CFUN: ",
                                                    &paras_count);
                /* +CFUN: <fun> */
                if (p_para_arr && paras_count >= 1) {
                    *p_status = atoi((u8 *)p_para_arr[0]);
                    app_logi("[%s]*p_status = %d.\r\n", __func__, *p_status);
                }
                aic_free_paras_mem(p_para_arr, paras_count);
            }
        }
        aic_free_paras_mem(p_line_arr, resp_to_save.resp_line_cnt);
    }

    app_logi("[%s]exit.(*p_status = %d)\r\n", __func__, *p_status);
    aic_free(resp_to_save.resp_buf);
    return 0;
}

void test_cfun_api(void)
{
    int ret = -1;
    uint8_t cfun_status = 0;

    ret = radio_set_cfun_status_sync(0, 4);
    app_logi("[%s] aic_srv_tele_radio_set_cfun_status_sync return: (%d)\n", __func__, ret);

    ret = radio_get_cfun_status_sync(0, &cfun_status);
    app_logi("[%s] aic_srv_tele_radio_get_cfun_status_sync return: (%d), cfun_status: (%d)\n", __func__, ret, cfun_status);
}
