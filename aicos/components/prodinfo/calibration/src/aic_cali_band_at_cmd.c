/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic band at Application Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x191077
#define LOG_MODULE    AMOD_PRODINFO

#include <string.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <stdio.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include <subsys_api.h>
#include <aic_at_interact_cp.h>
#include "aic_cali_band_cmd.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define CALI_BAND_ACQUIRE_MUTEX(mutex_ptr, wait_timeout) \
if (AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    u32 mutex_status; \
    AIC_ASSERT(NULL != mutex_ptr); \
    mutex_status = aos_get_mutex(mutex_ptr, wait_timeout); \
    AIC_ASSERT(AOS_SUCCESS == mutex_status); \
}

#define CALI_BAND_RELEASE_MUTEX(mutex_ptr) \
if (AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    AIC_ASSERT(NULL != mutex_ptr); \
    aos_put_mutex(mutex_ptr); \
}

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
aos_semaphore_ptr s_band_semp_ptr;
aos_mutex_ptr s_band_mutex_ptr;
struct lte_cali_band_info *s_lte_band_info = NULL;
struct lte_cali_afc_info s_lte_afc_info;
struct lte_cali_time_info s_lte_calitime_info;
u16 s_lte_cali_toolversion = 0;
u8 *s_rfid_name_info = NULL;
u8 *s_hw_diff_info = NULL;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_band_diag_handle - band diag handle.
 *  diag cmd:
 *  magic | sn | sys | errno | cmd | subcmd | len | idx | data | tail
 *
 * Parameter:
 * @diag_cmd: receive diag parameter.
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_band_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    AIC_ASSERT(diag_cmd->err_no == 0);

    switch (diag_cmd->subcmd) {
        case BAND_DIAG_READ_SUBCMD:
            aos_memcpy((u8 *)s_lte_band_info, (u8 *)diag_cmd->data,
                       diag_cmd->data_len);
            break;
        case AFC_DIAG_READ_SUBCMD:
            aos_memcpy((u8 *)&s_lte_afc_info, (u8 *)diag_cmd->data,
                       diag_cmd->data_len);
            break;
        case RFID_DIAG_READ_SUBCMD:
            aos_memcpy((u8 *)s_rfid_name_info, (u8 *)diag_cmd->data,
                       diag_cmd->data_len);
            break;
        case CALITIME_DIAG_READ_SUBCMD:
            aos_memcpy((u8 *)&s_lte_calitime_info, (u8 *)diag_cmd->data,
                       diag_cmd->data_len);
            break;
        case TOOLVER_DIAG_READ_SUBCMD:
            aos_memcpy((u8 *)&s_lte_cali_toolversion, (u8 *)diag_cmd->data,
                       diag_cmd->data_len);
            break;
        case HWDIFF_DIAG_READ_SUBCMD:
            s_hw_diff_info = aos_zalloc(diag_cmd->data_len + 4);
            AIC_ASSERT(s_hw_diff_info);

            aos_memcpy(s_hw_diff_info, (u8 *)diag_cmd->data,
                       diag_cmd->data_len);
            break;
        default:
            AIC_ASSERT(0);
            break;
    }

    aos_put_semaphore(s_band_semp_ptr);
}

/**
 * aic_band_is_cali - band is calibration or not.
 *
 * Parameter:
 *  @band_num: input band number.
 *
 * Return:
 */
boolean aic_band_is_cali(u8 band_num)
{
    boolean cali_flag = false;
    aos_status status;
    u8 i;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        aicx_error(0x085756, "", "cp not alive.\n");
        return false;
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    s_lte_band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                                 LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(s_lte_band_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, BAND_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
        if ((s_lte_band_info[i].band_num == band_num) ||
            (s_lte_band_info[i].coband0_num == band_num) ||
            (s_lte_band_info[i].coband1_num == band_num) ||
            (s_lte_band_info[i].coband2_num == band_num) ||
            (s_lte_band_info[i].coband3_num == band_num)) {
            if ((s_lte_band_info[i].cali_flag & LTE_CALI_PASS_FLAG) == LTE_CALI_PASS_FLAG) {
                cali_flag = true;
            }
            break;
        }
    }

    aos_free(s_lte_band_info);
    s_lte_band_info = NULL;

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return cali_flag;
}

/**
 * aic_calitime_get - get calibration time and toolversion.
 *
 * Parameter:
 *  @calitime: input calibration time ptr.
 *  @toolversion: input to store calibration version ptr.
 *
 * Return:
 */
void aic_calitime_get(struct lte_cali_time_info *calitime, u16 *toolversion)
{
    aos_status status;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        aicx_error(0x085756, "", "cp not alive.\n");
        return;
    }

    aos_memset(&s_lte_calitime_info, 0x0, sizeof(s_lte_calitime_info));
    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, CALITIME_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    s_lte_cali_toolversion = 0;
    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, TOOLVER_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    aos_memcpy(calitime, &s_lte_calitime_info, sizeof(s_lte_calitime_info));
    *toolversion = s_lte_cali_toolversion;
}

/**
 * aic_caliband_get - get all calibration band.
 *
 * Parameter:
 *  @band_num: input to store band number ptr.
 *  @count: input store band number count.
 *
 * Return:
 */
void aic_caliband_get(u8 *band_num, u8 count)
{
    aos_status status;
    u8 i, j;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        aicx_error(0x085756, "", "cp not alive.\n");
        return;
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    s_lte_band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                                 LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(s_lte_band_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, BAND_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    aos_memset(band_num, 0, count);

    for (i = 0, j = 0; i < LTE_CALI_BAND_INDEX_NUM && j < count; i++) {
        if ((s_lte_band_info[i].cali_flag & LTE_CALI_PASS_FLAG) == LTE_CALI_PASS_FLAG) {
            band_num[j++] = s_lte_band_info[i].band_num;
            if (j >= count)
                break;

            if (s_lte_band_info[i].coband0_num) {
                band_num[j++] = s_lte_band_info[i].coband0_num;
                if (j >= count)
                    break;
            }

            if (s_lte_band_info[i].coband1_num) {
                band_num[j++] = s_lte_band_info[i].coband1_num;
                if (j >= count)
                    break;
            }

            if (s_lte_band_info[i].coband2_num) {
                band_num[j++] = s_lte_band_info[i].coband2_num;
                if (j >= count)
                    break;
            }

            if (s_lte_band_info[i].coband3_num) {
                band_num[j++] = s_lte_band_info[i].coband3_num;
                if (j >= count)
                    break;
            }
        }
    }

    aos_free(s_lte_band_info);
    s_lte_band_info = NULL;

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);
}

/**
 * aic_band_is_nst - band is nst pass or not.
 *
 * Parameter:
 *  @band_num: input band number.
 *
 * Return:
 */
boolean aic_band_is_nst(u8 band_num)
{
    boolean cali_flag = false;
    aos_status status;
    u8 i;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        aicx_error(0x085756, "", "cp not alive.\n");
        return false;
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    s_lte_band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                                 LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(s_lte_band_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, BAND_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
        if ((s_lte_band_info[i].band_num == band_num) ||
            (s_lte_band_info[i].coband0_num == band_num) ||
            (s_lte_band_info[i].coband1_num == band_num) ||
            (s_lte_band_info[i].coband2_num == band_num) ||
            (s_lte_band_info[i].coband3_num == band_num)) {
            if ((s_lte_band_info[i].cali_flag & LTE_NST_PASS_FLAG) == LTE_NST_PASS_FLAG) {
                cali_flag = true;
            }
            break;
        }
    }

    aos_free(s_lte_band_info);
    s_lte_band_info = NULL;

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return cali_flag;
}

/**
 * aic_nstband_get - get all nst pass band.
 *
 * Parameter:
 *  @band_num: input to store band number ptr.
 *  @count: input store band number count.
 *
 * Return:
 */
void aic_nstband_get(u8 *band_num, u8 count)
{
    aos_status status;
    u8 i, j;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        aicx_error(0x085756, "", "cp not alive.\n");
        return;
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    s_lte_band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                                 LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(s_lte_band_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, BAND_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    aos_memset(band_num, 0, count);

    for (i = 0, j = 0; i < LTE_CALI_BAND_INDEX_NUM && j < count; i++) {
        if ((s_lte_band_info[i].cali_flag & LTE_NST_PASS_FLAG) == LTE_NST_PASS_FLAG) {
            band_num[j++] = s_lte_band_info[i].band_num;
            if (j >= count)
                break;

            if (s_lte_band_info[i].coband0_num) {
                band_num[j++] = s_lte_band_info[i].coband0_num;
                if (j >= count)
                    break;
            }

            if (s_lte_band_info[i].coband1_num) {
                band_num[j++] = s_lte_band_info[i].coband1_num;
                if (j >= count)
                    break;
            }

            if (s_lte_band_info[i].coband2_num) {
                band_num[j++] = s_lte_band_info[i].coband2_num;
                if (j >= count)
                    break;
            }

            if (s_lte_band_info[i].coband3_num) {
                band_num[j++] = s_lte_band_info[i].coband3_num;
                if (j >= count)
                    break;
            }
        }
    }

    aos_free(s_lte_band_info);
    s_lte_band_info = NULL;

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);
}

/**
 * aic_afc_caliinfo_get - get afc calibration information.
 *
 * Parameter:
 *  @afc_info: input afc information ptr.
 *
 * Return:
 */
void aic_afc_caliinfo_get(struct lte_cali_afc_info *afc_info)
{
    aos_status status;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        aicx_error(0x085756, "", "cp not alive.\n");
        return;
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, AFC_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    aos_memcpy(afc_info, &s_lte_afc_info, sizeof(struct lte_cali_afc_info));

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);
}

/**
 * aic_rfid_name_get - get rfid name.
 *
 * Parameter:
 *  @hwid_name: input hwid name ptr, length = HW_ID_NAME_LEN.
 *  @rfid_name: input rfid name ptr, length = RF_ID_NAME_LEN;
 *
 * Return:
 */
void aic_rfid_name_get(char *hwid_name, char *rfid_name)
{
    aos_status status;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        aicx_error(0x085756, "", "cp not alive.\n");
        return;
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    s_rfid_name_info = aos_zalloc(HW_ID_NAME_LEN + RF_ID_NAME_LEN);
    AIC_ASSERT(s_rfid_name_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, RFID_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    aos_memset(hwid_name, 0x0, HW_ID_NAME_LEN);
    aos_memset(rfid_name, 0x0, RF_ID_NAME_LEN);

    snprintf(hwid_name, HW_ID_NAME_LEN, "hw_%02d", *s_rfid_name_info);
    snprintf(rfid_name, RF_ID_NAME_LEN, "%s", s_rfid_name_info + 1);

    if (s_rfid_name_info) {
        aos_free(s_rfid_name_info);
        s_rfid_name_info = NULL;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);
}

/**
 * aic_afc_is_cali - afc is calibration or not.
 *
 * Parameter:
 *
 * Return:
 */
boolean aic_afc_is_cali(void)
{
    struct lte_cali_afc_info afc_info;

    aic_afc_caliinfo_get(&afc_info);

    if (afc_info.cali_flag)
        return true;

    return false;
}

/**
 * aic_band_afc_query_cmd - query AFC information.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_afc_at_query_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct lte_cali_afc_info afc_info;
    int errcode;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        line_pointers[0] = (ulong)result_line;
        return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    aic_afc_caliinfo_get(&afc_info);

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+AFC= %d,%d,%d,%d,%d,%d",
             afc_info.cali_flag, afc_info.cali_freq,
             afc_info.ct, afc_info.afc_cal, afc_info.hpm_32k,
             afc_info.lpm_32k);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;

    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);

    return AIC_SUCCESS;
}

/**
 * fs_at_caliband_cmd - response AT+CALIBAND=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 aic_caliband_at_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+CALIBAND: <band_num>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * aic_caliband_at_get - process at+CALIBAND?.
 *
 * Parameter:
 *  @rsp_str: at response.
 *  @rsp_str: response length.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 *
 * 获取RF校准状态 added by huang.junkai
 */
u32 aic_caliband_at_get(char *rsp_str, u32 rsp_len)
{
    u16 cali_flag;
    u8 i;
    aos_status status;

    aos_memset(rsp_str, 0x0, rsp_len);
    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        u8 result_line1[AT_MAX_AP_RESP_LENGTH] = {'\0'};
        snprintf(rsp_str, rsp_len,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        return AT_ERROR_INVALID_RESP;
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    s_lte_band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                                 LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(s_lte_band_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, BAND_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    aos_memset(&s_lte_calitime_info, 0x0, sizeof(s_lte_calitime_info));
    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, CALITIME_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    s_lte_cali_toolversion = 0;
    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, TOOLVER_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    cali_flag = 0;
    snprintf(rsp_str, rsp_len, "+CALIBAND= %04d%02d%02d%02d%02d%02d,%d,",
            s_lte_calitime_info.year, s_lte_calitime_info.mon, s_lte_calitime_info.day,
            s_lte_calitime_info.hour, s_lte_calitime_info.min, s_lte_calitime_info.sec,
            s_lte_cali_toolversion);
    for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
        if ((s_lte_band_info[i].cali_flag & LTE_CALI_PASS_FLAG) == LTE_CALI_PASS_FLAG) {
            snprintf(rsp_str + strlen(rsp_str),
                     rsp_len - strlen(rsp_str),
                     "%d,", s_lte_band_info[i].band_num);
            if (s_lte_band_info[i].coband0_num)
                snprintf(rsp_str + strlen(rsp_str),
                         rsp_len - strlen(rsp_str),
                         "%d,", s_lte_band_info[i].coband0_num);

            if (s_lte_band_info[i].coband1_num)
                snprintf(rsp_str + strlen(rsp_str),
                         rsp_len - strlen(rsp_str),
                         "%d,", s_lte_band_info[i].coband1_num);

            if (s_lte_band_info[i].coband2_num)
                snprintf(rsp_str + strlen(rsp_str),
                         rsp_len - strlen(rsp_str),
                         "%d,", s_lte_band_info[i].coband2_num);

            if (s_lte_band_info[i].coband3_num)
                snprintf(rsp_str + strlen(rsp_str),
                         rsp_len - strlen(rsp_str),
                         "%d,", s_lte_band_info[i].coband3_num);

            cali_flag = 1;
        }
    }

    if (s_lte_band_info) {
        aos_free(s_lte_band_info);
        s_lte_band_info = NULL;
    }

    if (cali_flag) {
        rsp_str[strlen(rsp_str) - 1] = '\0';
        return AIC_SUCCESS;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);
    aos_memset(rsp_str, 0x0, rsp_len);

    return AT_ERROR_INVALID_RESP;
}

/**
 * aic_caliband_at_query_cmd - process at+CALIBAND?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_caliband_at_query_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u16 cali_flag, result_len, line_count;
    u8 i, *result_line;
    aos_status status;
    int errcode;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        u8 result_line1[AT_MAX_AP_RESP_LENGTH] = {'\0'};
        snprintf(result_line1, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        line_pointers[0] = (ulong)result_line1;
        return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    result_len = 512;
    line_count = 0;

    result_line = aos_zalloc(result_len);

    s_lte_band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                                 LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(s_lte_band_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, BAND_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    aos_memset(&s_lte_calitime_info, 0x0, sizeof(s_lte_calitime_info));
    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, CALITIME_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    s_lte_cali_toolversion = 0;
    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, TOOLVER_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    cali_flag = 0;
    snprintf(result_line, result_len, "+CALIBAND= %04d%02d%02d%02d%02d%02d,%d,",
            s_lte_calitime_info.year, s_lte_calitime_info.mon, s_lte_calitime_info.day,
            s_lte_calitime_info.hour, s_lte_calitime_info.min, s_lte_calitime_info.sec,
            s_lte_cali_toolversion);
    for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
        if ((s_lte_band_info[i].cali_flag & LTE_CALI_PASS_FLAG) == LTE_CALI_PASS_FLAG) {
            snprintf(result_line + strlen(result_line),
                     result_len - strlen(result_line),
                     "%d,", s_lte_band_info[i].band_num);
            if (s_lte_band_info[i].coband0_num)
                snprintf(result_line + strlen(result_line),
                         result_len - strlen(result_line),
                         "%d,", s_lte_band_info[i].coband0_num);

            if (s_lte_band_info[i].coband1_num)
                snprintf(result_line + strlen(result_line),
                         result_len - strlen(result_line),
                         "%d,", s_lte_band_info[i].coband1_num);

            if (s_lte_band_info[i].coband2_num)
                snprintf(result_line + strlen(result_line),
                         result_len - strlen(result_line),
                         "%d,", s_lte_band_info[i].coband2_num);

            if (s_lte_band_info[i].coband3_num)
                snprintf(result_line + strlen(result_line),
                         result_len - strlen(result_line),
                         "%d,", s_lte_band_info[i].coband3_num);

            cali_flag = 1;
        }
    }

    if (cali_flag) {
        result_line[strlen(result_line) - 1] = '\0';
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    }

    line_pointers[line_count++] = (ulong)AT_RESULT_OK;

    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    if (result_line)
        aos_free(result_line);

    if (s_lte_band_info) {
        aos_free(s_lte_band_info);
        s_lte_band_info = NULL;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return errcode;
}

/**
 * aic_caliband_at_set_cmd - process at+CALIBAND=band_num.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_caliband_at_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u16 cali_flag, count, line_count;
    aos_status status;
    ulong *paras_arr;
    u8 band_num, i;
    int errcode;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        line_pointers[0] = (ulong)result_line;
        return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    do {
        line_count = 0;

        errcode = AIC_SUCCESS;
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+CALIBAND=",
                                                 &count);
        if (count != 1 || !paras_arr) {
            errcode = AIC_EINVAL;
            break;
        }

        s_lte_band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                                     LTE_CALI_BAND_INDEX_NUM);
        AIC_ASSERT(s_lte_band_info);

        ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, BAND_DIAG_READ_SUBCMD, NULL, 0);
        status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
        AIC_ASSERT(status == AOS_SUCCESS);

        cali_flag = 0;
        band_num = atoi((u8 *)paras_arr[0]);
        for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
            if ((s_lte_band_info[i].band_num == band_num) ||
                (s_lte_band_info[i].coband0_num == band_num) ||
                (s_lte_band_info[i].coband1_num == band_num) ||
                (s_lte_band_info[i].coband2_num == band_num) ||
                (s_lte_band_info[i].coband3_num == band_num)) {
                cali_flag = s_lte_band_info[i].cali_flag;
                break;
            }
        }

        if (i == LTE_CALI_BAND_INDEX_NUM) {
            errcode = AIC_EINVAL;
            break;
        }

        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CALIBAND= %d,%d", band_num, cali_flag);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", -errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[line_count++] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    if (s_lte_band_info) {
        aos_free(s_lte_band_info);
        s_lte_band_info = NULL;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return errcode;
}

/**
 * fs_at_nstband_cmd - response AT+NSTBAND=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 aic_nstband_at_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+NSTBAND: <band_num>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * aic_nstband_at_query_cmd - process at+NSTBAND?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_nstband_at_query_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u16 cali_flag, result_len, line_count;
    u8 i, *result_line;
    aos_status status;
    int errcode;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        u8 result_line1[AT_MAX_AP_RESP_LENGTH] = {'\0'};
        snprintf(result_line1, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        line_pointers[0] = (ulong)result_line1;
        return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    result_len = 512;
    line_count = 0;

    result_line = aos_zalloc(result_len);

    s_lte_band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                                 LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(s_lte_band_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, BAND_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    cali_flag = 0;
    snprintf(result_line, result_len, "+NSTBAND=");
    for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
        if ((s_lte_band_info[i].cali_flag & LTE_NST_PASS_FLAG) == LTE_NST_PASS_FLAG) {
            snprintf(result_line + strlen(result_line),
                     result_len - strlen(result_line),
                     "%d,", s_lte_band_info[i].band_num);
            if (s_lte_band_info[i].coband0_num)
                snprintf(result_line + strlen(result_line),
                         result_len - strlen(result_line),
                         "%d,", s_lte_band_info[i].coband0_num);

            if (s_lte_band_info[i].coband1_num)
                snprintf(result_line + strlen(result_line),
                         result_len - strlen(result_line),
                         "%d,", s_lte_band_info[i].coband1_num);

            if (s_lte_band_info[i].coband2_num)
                snprintf(result_line + strlen(result_line),
                         result_len - strlen(result_line),
                         "%d,", s_lte_band_info[i].coband2_num);

            if (s_lte_band_info[i].coband3_num)
                snprintf(result_line + strlen(result_line),
                         result_len - strlen(result_line),
                         "%d,", s_lte_band_info[i].coband3_num);

            cali_flag = 1;
        }
    }

    if (cali_flag) {
        result_line[strlen(result_line) - 1] = '\0';
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    }

    line_pointers[line_count++] = (ulong)AT_RESULT_OK;

    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    if (result_line)
        aos_free(result_line);

    if (s_lte_band_info) {
        aos_free(s_lte_band_info);
        s_lte_band_info = NULL;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return errcode;
}

/**
 * aic_rfid_at_cmd - process AT+RFID=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_rfid_at_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    aos_status status;
    u8 *ptr;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        line_pointers[0] = (ulong)result_line;
        return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, HWDIFF_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    ptr = s_hw_diff_info;
    while(1) {
        if (strlen(ptr)) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+RFID: %s", ptr);
        } else {
            break;
        }
        ptr += HW_ID_NAME_LEN;

        if (strlen(ptr)) {
            snprintf(result_line + strlen(result_line),
                    AT_MAX_AP_RESP_LENGTH - strlen(result_line),
                    ",%s", ptr);
        }
        ptr += RF_ID_NAME_LEN;

        line_pointers[0] = (ulong)result_line;
        aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    line_pointers[0] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);

    if (s_hw_diff_info) {
        aos_free(s_hw_diff_info);
        s_hw_diff_info = NULL;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return AIC_SUCCESS;
}

/**
 * aic_rfid_at_query_cmd - process AT+RFID?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_rfid_at_query_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    aos_status status;
    u16 line_count;
    int errcode;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        line_pointers[0] = (ulong)result_line;
        return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    s_rfid_name_info = aos_zalloc(HW_ID_NAME_LEN + RF_ID_NAME_LEN);
    AIC_ASSERT(s_rfid_name_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, RFID_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    if (*s_rfid_name_info != 0) {
        if (strlen(s_rfid_name_info + 1) == 0) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                    "+RFID: hw_%02d", *s_rfid_name_info);
        } else {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                    "+RFID: hw_%02d,%s", *s_rfid_name_info, s_rfid_name_info + 1);
        }
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    if (s_rfid_name_info) {
        aos_free(s_rfid_name_info);
        s_rfid_name_info = NULL;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return errcode;
}

/**
 * aic_mrfinfo_at_cmd - process AT+MRFINFO=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_mrfinfo_at_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    aos_status status;
    u8 *ptr;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        line_pointers[0] = (ulong)result_line;
        return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, HWDIFF_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    ptr = s_hw_diff_info;
    while(1) {
        if (strlen(ptr)) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+MRFINFO: %s", ptr);
        } else {
            break;
        }
        ptr += HW_ID_NAME_LEN;

        if (strlen(ptr)) {
            snprintf(result_line + strlen(result_line),
                    AT_MAX_AP_RESP_LENGTH - strlen(result_line),
                    ",%s", ptr);
        }
        ptr += RF_ID_NAME_LEN;

        line_pointers[0] = (ulong)result_line;
        aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    line_pointers[0] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);

    if (s_hw_diff_info) {
        aos_free(s_hw_diff_info);
        s_hw_diff_info = NULL;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return AIC_SUCCESS;
}

/**
 * aic_mrfinfo_at_query_cmd - process AT+MRFINFO?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_mrfinfo_at_query_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    aos_status status;
    u16 line_count;
    int errcode;

    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_CP_NOT_ALIVE);
        line_pointers[0] = (ulong)result_line;
        return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    }

    CALI_BAND_ACQUIRE_MUTEX(s_band_mutex_ptr, AOS_WAIT_FOREVER);

    s_rfid_name_info = aos_zalloc(HW_ID_NAME_LEN + RF_ID_NAME_LEN);
    AIC_ASSERT(s_rfid_name_info);

    ap_send_diag_cmd_to_cp(DIAG_CMD_BAND, RFID_DIAG_READ_SUBCMD, NULL, 0);
    status = aos_get_semaphore(s_band_semp_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(status == AOS_SUCCESS);

    if (*s_rfid_name_info != 0) {
        if (strlen(s_rfid_name_info + 1) == 0) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                    "+MRFINFO: hw_%02d", *s_rfid_name_info);
        } else {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                    "+MRFINFO: hw_%02d,%s", *s_rfid_name_info, s_rfid_name_info + 1);
        }
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    if (s_rfid_name_info) {
        aos_free(s_rfid_name_info);
        s_rfid_name_info = NULL;
    }

    CALI_BAND_RELEASE_MUTEX(s_band_mutex_ptr);

    return errcode;
}

/*
 *  Add band AT handle here.
 */
const static AT_CMD_HANDLE aic_band_at_handles[] =
{
    {
        "AT+AFC",
        NULL,
        aic_afc_at_query_cmd,
        NULL,
        NULL
    },
    {
        "AT+CALIBAND",
        aic_caliband_at_cmd,
        aic_caliband_at_query_cmd,
        aic_caliband_at_set_cmd,
        NULL
    },
    {
        "AT+NSTBAND",
        aic_nstband_at_cmd,
        aic_nstband_at_query_cmd,
        NULL,
        NULL
    },
    {
        "AT+RFID",
        aic_rfid_at_cmd,
        aic_rfid_at_query_cmd,
        NULL,
        NULL
    },
    {
        "AT+MRFINFO",
        aic_mrfinfo_at_cmd,
        aic_mrfinfo_at_query_cmd,
        NULL,
        NULL
    },
    {AIC_NULL},
};

/**
 * aic_band_at_cmd_init - band at and diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_band_at_cmd_init(void)
{
    s_band_semp_ptr = aos_create_semaphore("CALIBAND", 0);
    AIC_ASSERT(s_band_semp_ptr);

    s_band_mutex_ptr = aos_create_mutex("CALIBAND");
    AIC_ASSERT(s_band_mutex_ptr);

    diag_register_cmd_handler(DIAG_CMD_BAND, aic_band_diag_handle, NULL);

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)aic_band_at_handles);

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(aic_band_at_cmd_init);

