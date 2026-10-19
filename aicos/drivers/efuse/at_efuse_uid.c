/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is efuse at cmd handle.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b289b
#define LOG_MODULE    AMOD_DRV
#include <aic_osa.h>
#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <cache.h>
#include <efuse.h>

#define EFUSE_MAX_AT_RESP_LENGTH        64UL
#define EFUSE_REPLY_LINE_COUNT_ONCE     16UL

u32 aic_efuse_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[EFUSE_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[EFUSE_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, EFUSE_MAX_AT_RESP_LENGTH, "+efuse");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 aic_efuse_getuid_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u32 ret;
    int len;

    u8 result_line0[50] = {'\0'};
    ulong line_pointers[EFUSE_REPLY_LINE_COUNT_ONCE];

    ret = aic_get_uid(result_line0, &len);

    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return ret;
}

u32 aic_chipid_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[EFUSE_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[EFUSE_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, EFUSE_MAX_AT_RESP_LENGTH, "+chipid");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 aic_get_chipid_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[EFUSE_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[EFUSE_REPLY_LINE_COUNT_ONCE];
    u32 chipid;

    chipid = AIC_READ32(REG_CHIP_ID);
    snprintf(result_line0, EFUSE_MAX_AT_RESP_LENGTH, "+chipid:0x%x", chipid);
    strcpy(result_line1, "OK");

    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

AT_CMD_HANDLE efuse_at_handles[] = {
    {"AT+GETUID", aic_efuse_test_cmd, AIC_NULL,
     aic_efuse_getuid_cmd, AIC_NULL},
    {"AT+CHIPID", aic_chipid_test_cmd, AIC_NULL,
     aic_get_chipid_cmd, AIC_NULL},
    {AIC_NULL},
};

int efuse_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&efuse_at_handles[0]);
    return 0;
}
AIC_COMP_L2_INIT_EXPORT(efuse_at_cmd_init);
