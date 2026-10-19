/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This system cmd realize file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14e17c
#define LOG_MODULE    AMOD_INIT

#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_boot_mode.h>
#include <aic_init.h>
#include <aipc_api.h>

#define SUBSYS_CP_ALIVE        "ALIVE: cp sys."

int sys_misc_init(void)
{
#ifdef CONFIG_SUBSYS_CP
    ABRR_HANDLE brr;
    ABLK_T blk;
    int size;

    /* send SUBSYS_CP_ALIVE to ap. */
    brr = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_COMM);
    if (brr) {
        size = strlen(SUBSYS_CP_ALIVE) + 1;
        if (AIC_SUCCESS == ablk_get_block(brr, size, &blk, AIPC_NO_WAIT)) {
            memcpy(blk.addr, SUBSYS_CP_ALIVE, size);
            blk.size = size;
            if (ablk_send_block(brr, STATUS_LINK, &blk, AIPC_NO_WAIT))
                ablk_recycle_block(brr, &blk);
        }
        ablk_release_brr(brr);
    }
#endif

    return AIC_SUCCESS;
}
AIC_COMP_P2_INIT_EXPORT(sys_misc_init);

#ifdef CONFIG_SUBSYS_AP
#include <aic_at_resp_handle.h>
#include <aic_at_interact_cp.h>

static u32 atsys_blt_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    char * param;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^BLT=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    param = (char *)paras_arr[0];
    if (param[0] != '1') {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (aic_boot_mode_is_charge()) {
        aic_boot_mode_set(BOOT_MODE_NORMAL);
        aic_init_charge_dis();
        aic_send_cfun_cmd(AT_START_CP);
    }

    rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}
/*
 *  Add sys AT handle here.
 */
const static AT_CMD_HANDLE sys_at_handles[] =
{
    {
        "AT^BLT",
        AIC_NULL,
        AIC_NULL,
        atsys_blt_set_cmd,
        AIC_NULL
    },

    {
        AIC_NULL
    },
};

int aic_sys_at_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)sys_at_handles);

    return AIC_SUCCESS;
}
AIC_COMP_P2_INIT_EXPORT(aic_sys_at_init);
#endif

