/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  pmu at operate interface implementation.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x120be0
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
#include <pmu_user.h>
#include <pmu_driver.h>
#include <timer_inc.h>

#define PMU_USER_NAME   "PMIC_PMU_AT"

static u32 at_pmu_info_set_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+pmu_info_set: \"pmu_unit_name\", \"target_voltage_mv\", \"enable(0,1)\"");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_pmu_info_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct pmu_user *puser;
    int ret, voltage;
    ulong *paras_arr;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+PMU_INFO_SET=", &count);
    if (!paras_arr || count != 3)
        goto CMD_ERROR;

    puser = pmu_get(PMU_USER_NAME, (char *)paras_arr[0]);
    if (!puser) {
        ret = AIC_ENXIO;
        aicx_warn(0x12b6a9, "ss", "%s get pmu (%s) operate handler fail!\n", __func__, (char *)paras_arr[0]);
        goto CMD_ERROR;
    }

    voltage = atoi((u8 *)paras_arr[1]);

    ret = pmu_voltage_set(puser, voltage * 1000, puser->punit->pinfo->voltage_step_uV);
    if (ret) {
        aicx_warn(0x009d4d, "ssd", "%s set %s voltage %d mv fail!\n", __func__, paras_arr[0], paras_arr[1]);
        goto RELEASE_HANDLE;
    }

    if (!pmu_is_enabled(puser) && paras_arr[2]) {
        ret = pmu_enable(puser);
        if (ret)
            aicx_warn(0x1948ea, "ss", "%s %s enable fail!\n", __func__, paras_arr[0]);
    } else if (pmu_is_enabled(puser) && !paras_arr[2]) {
        ret = pmu_disable(puser);
        if (ret)
            aicx_warn(0x191d20, "ss", "%s %s disable fail!\n", __func__, paras_arr[0]);
    }

    if (ret)
        goto RELEASE_HANDLE;

    pmu_put(puser);

    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    /* Fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;

RELEASE_HANDLE:
    pmu_put(puser);

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_CMD);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

static u32 at_pmu_info_get_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+pmu_info_get: \"pmu_unit_name\"");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_pmu_info_get_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct pmu_user *puser;
    u32 voltage;
    int ret, enabled;
    ulong *paras_arr;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+PMU_INFO_GET=", &count);
    if (!paras_arr || count != 1)
        goto CMD_ERROR;

    puser = pmu_get(PMU_USER_NAME, (char *)paras_arr[0]);
    if (!puser) {
        ret = AIC_ENXIO;
        aicx_warn(0x12b6a9, "ss", "%s get pmu (%s) operate handler fail!\n", __func__, (char *)paras_arr[0]);
        goto CMD_ERROR;
    }

    ret = pmu_voltage_get(puser, &voltage);
    if (ret) {
        aicx_warn(0x0973ac, "ss", "%s get %s voltage fail!\n", __func__, paras_arr[0]);
        goto RELEASE_HANDLE;
    }

    enabled = pmu_is_enabled(puser);

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+PMU_INFO_GET:%s,%d,%d", (char *)paras_arr[0], voltage / 1000, enabled);

    pmu_put(puser);

    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    /* Fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;

RELEASE_HANDLE:
    pmu_put(puser);

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_CMD);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}


AT_CMD_HANDLE pmu_at_handles[] = {
    {
        "AT+PMU_INFO_SET",
        at_pmu_info_set_test_cmd,
        AIC_NULL,
        at_pmu_info_set_cmd,
        AIC_NULL
    },
    {
        "AT+PMU_INFO_GET",
        at_pmu_info_get_test_cmd,
        AIC_NULL,
        at_pmu_info_get_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};

int pmu_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&pmu_at_handles[0]);

    return 0;
}
AIC_APP_L1_INIT_EXPORT(pmu_at_cmd_init);
