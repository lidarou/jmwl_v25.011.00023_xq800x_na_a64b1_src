/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  pmic efuse at operate interface implementation.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0967b5
#define LOG_MODULE    AMOD_PMIC
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_at_resp_handle.h>
#include <aic_bitmap.h>
#include <pmic_efuse.h>
#include <timer_inc.h>

static u32 pmic_efuse_set_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+PMIC_EFUSE: <blk_num: 0-3>,<blk_value: u32>");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 pmic_efuse_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u8 result_line1[5] = {'\0'};
    u32 blk_num, blk_val;
    ulong *paras_arr;
    u16 count;
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "at+pmic_efuse=", &count);
    if (!paras_arr || count != 2)
        goto CMD_ERROR;

    blk_num = strtoul((char *)paras_arr[0], NULL, 16);
    blk_val = strtoul((char *)paras_arr[1], NULL, 16);

    ret = aic_pmic_efuse_block_write(blk_num, blk_val);
    if (ret)
        goto CMD_ERROR;

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+pmic_efuse: blk:%d, val:0x%x", blk_num, blk_val);
    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    /* Fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_CMD);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

#define PMIC_EFUSE_BLK_NUM_MAX 4

static u32 pmic_efuse_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u32 val;
    int i, ret;

    for ( i = 0; i < PMIC_EFUSE_BLK_NUM_MAX; i ++) {

        ret = aic_pmic_efuse_block_read(i, &val);
        if (ret)
            snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+PMIC_EFUSE: bld:%d, read failed!", i);
        else
            snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+PMIC_EFUSE: bld:%d, val:0x%x", i, val);

        /* Fill response,one line. */
        line_pointers[0] = (ulong)&result_line0[0];
        aos_usleep(1000);//add for trigger at flush buf to send.
        aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    }

    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

}

AT_CMD_HANDLE pmic_efuse_at_handles[] = {
    {
        "AT+PMIC_EFUSE",
        pmic_efuse_set_test_cmd,
        pmic_efuse_query_cmd,
        pmic_efuse_set_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};

int pmic_efuse_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&pmic_efuse_at_handles[0]);

    return 0;
}
AIC_APP_L1_INIT_EXPORT(pmic_efuse_at_cmd_init);
