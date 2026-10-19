/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1b3ef4
#define LOG_MODULE    AMOD_CLK_CALI

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_type.h>
#include <aipc_api.h>
#include <pmic_io.h>
#include <pmic_reg_base.h>
#include <glb_rtc.h>
#include <glb_pd.h>
#include "pmu_rtc.h"
#include <timer_inc.h>

/* DCXO CALI MSG TYPE */
#define MSG_DCXO_CALI_DIAG_ENTER_LP_MODE     (0x1)
#define MSG_DCXO_CALI_DIAG_ENTER_HP_MODE     (0x2)
#define MSG_DCXO_CALI_LP_ACK                 (0x3)
#define MSG_DCXO_CALI_HP_ACK                 (0x4)
#define MSG_DCXO_CALI_TYPE_MAX               (0xFF)

/* DCXO CALI ACK */
#define MSG_DCXO_CALI_ACK_SUCCESS            (0x0)
#define MSG_DCXO_CALI_ACK_FAIL               (0x1)

/* DCXO cali event */
#define DCXO_CALI_EVENT_LP_SUCCESS           (0x1)
#define DCXO_CALI_EVENT_HP_SUCCESS           (0x2)

extern void lp_xc26m_pmic_ctrl(BOOL on);

/*
 * msg: amsg transfer from CP to AP.
 *      msg->p1: CLK_CALI MSG type
 *      msg->p2: CLK_CALI event
 */
static int _clk_cali_cp_receive_handle(AMSG_T *msg, void *p)
{
    int ret;
    AMSG_T send_msg;

    if (msg->p1 == MSG_DCXO_CALI_DIAG_ENTER_LP_MODE) {
        /* call 26m enter lp interface */
        lp_xc26m_pmic_ctrl(FALSE);
        /* send lp mode success msg */
        amsg_fill_msg(&send_msg, AMOD_CLK_CALI, MSG_DCXO_CALI_LP_ACK, MSG_DCXO_CALI_ACK_SUCCESS);
        ret = amsg_raw_send_msg(AIC_SYS_AP, &send_msg);
        AIC_ASSERT(ret == AIC_SUCCESS);
        return AIC_SUCCESS;
    } else if(msg->p1 == MSG_DCXO_CALI_DIAG_ENTER_HP_MODE){
        /* call 26m enter hp interface */
        lp_xc26m_pmic_ctrl(TRUE);
        /* send hp mode success msg */
        amsg_fill_msg(&send_msg, AMOD_CLK_CALI, MSG_DCXO_CALI_HP_ACK, MSG_DCXO_CALI_ACK_SUCCESS);
        ret = amsg_raw_send_msg(AIC_SYS_AP, &send_msg);
        AIC_ASSERT(ret == AIC_SUCCESS);
    } else {
        AIC_ASSERT(0);
    }

    return AIC_SUCCESS;
}

int aic_dcxo_cali_diag_init(void)
{
    AMSG_HANDLER_T receive_handle;
    u8 remote_sys;
    int ret;

    remote_sys = AIC_SYS_AP;
    receive_handle = _clk_cali_cp_receive_handle;
    ret = amsg_register_mod_handler(remote_sys, AMOD_CLK_CALI, receive_handle, NULL);
    AIC_ASSERT(ret == AIC_SUCCESS);

    return AIC_SUCCESS;
}

AIC_DRV_P2_INIT_EXPORT(aic_dcxo_cali_diag_init);

