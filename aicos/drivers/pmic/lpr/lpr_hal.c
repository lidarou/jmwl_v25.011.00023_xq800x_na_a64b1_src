/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of lpr HAL module
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x070e42
#define LOG_MODULE    AMOD_PMIC

#include <aic_errno.h>
#include <aic_type.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_resp_handle.h>
#include <aic_at_process_common.h>
#include <lpr.h>
#include "lpr_phy.h"
#include <pwr_key_cfg.h>

#define AIC_LPR_MUTEX      "LPR_MUTEX"
aos_mutex_ptr aic_lpr_mutex = AOS_NULL;

int lpr_config(BOOL enabled, u32 effect_time_s)
{
    int ret = 0;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_lpr_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    ret = lpr_phy_config(enabled, effect_time_s);
    if (ret)
        aicx_error(0x14b506, "", "lpr config failed!!\n");

    if (aos_is_user_thread_context())
        aos_put_mutex(aic_lpr_mutex);

    return ret;
}

int aic_lpr_init(void)
{
    aic_lpr_mutex = aos_create_mutex(AIC_LPR_MUTEX);
    AIC_ASSERT(aic_lpr_mutex);

    if (PWR_LPR_ENABLED) {
        lpr_config(true, PWR_LPR_TIMING);
    } else {
        disable_lpr();
    }

    return 0;
}
AIC_DRV_L1_INIT_EXPORT(aic_lpr_init);

#if defined (CONFIG_DRIVER_AT_LPR) && (CONFIG_SUBSYS_AP)
static u32 at_lpr_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^LPR");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "ON,effect_time_s / OFF.");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_lpr_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    ulong rsp[1];
    int ret = 0;
    u16 count;
    u32 effect_time = 0;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LPR=", &count);
    if (!paras_arr || count > 2) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (!strcmp((char *)paras_arr[0],"ON") || !strcmp((char *)paras_arr[0],"on")
        || !strcmp((char *)paras_arr[0],"On")) {
        effect_time = atoi((u8 *)paras_arr[1]);
        if (!effect_time) {
            ret = AIC_ENXIO;
        } else {
            ret = lpr_config(true, effect_time);
        }
    } else if (!strcmp((char *)paras_arr[0],"OFF") || !strcmp((char *)paras_arr[0],"off")
        || !strcmp((char *)paras_arr[0],"OFf")) {
        ret = lpr_config(false, effect_time);
    } else {
        ret = AIC_ENXIO;
    }

    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

AT_CMD_HANDLE lpr_at_handles[] = {
    {
        "AT^LPR",
        at_lpr_test_cmd,
        AIC_NULL,
        at_lpr_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};


int lpr_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&lpr_at_handles[0]);

    return 0;
}
AIC_COMP_L2_INIT_EXPORT(lpr_at_cmd_init);
#endif
