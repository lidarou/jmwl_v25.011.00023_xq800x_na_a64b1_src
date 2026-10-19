/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04f22b
#define LOG_MODULE    AMOD_CLK_CALI

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_type.h>
#include <aipc_api.h>
#include <diag_cmd_api.h>
#include <pmic_io.h>
#include <pmic_reg_base.h>
#include <glb_rtc.h>
#include <glb_pd.h>
#include "pmu_rtc.h"
#include <timer_inc.h>

#include <aic_cmdline.h>

#include <aic_md_id.h>
#include <aic_md_api.h>
#include "mdfile/inc/DCXOCALI.h"

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

typedef enum {
    DCXO_CALI_DIAG_ENTER_LP_MODE = 0,
    DCXO_CALI_DIAG_ENTER_HP_MODE,
    DCXO_CALI_DIAG_WRITE_CT,
    DCXO_CALI_DIAG_DIV_DCOEFF_HP_32K,
    DCXO_CALI_DIAG_DIV_DCOEFF_LP_32K,
    DCXO_CALI_DIAG_GET_26M_TYPE,
} DCXO_CALI_DIAG_SUBCMD;

aos_event_ptr dcxo_cali_event = AIC_NULL;

/*
 * msg: amsg transfer from CP to AP.
 *      msg->p1: CLK_CALI MSG type
 *      msg->p2: CLK_CALI event
 */
static int _clk_cali_ap_receive_handle(AMSG_T *msg, void *p)
{
    if (msg->p1 == MSG_DCXO_CALI_LP_ACK) {
        if (msg->p2 == MSG_DCXO_CALI_ACK_SUCCESS) {
            aos_set_event(dcxo_cali_event, DCXO_CALI_EVENT_LP_SUCCESS, AOS_OR);
        } else {
            AIC_ASSERT(0);
        }
    } else if(msg->p1 == MSG_DCXO_CALI_HP_ACK){
        if (msg->p2 == MSG_DCXO_CALI_ACK_SUCCESS) {
            aos_set_event(dcxo_cali_event, DCXO_CALI_EVENT_HP_SUCCESS, AOS_OR);
        } else {
            AIC_ASSERT(0);
        }
    } else {
        AIC_ASSERT(0);
    }

    return AIC_SUCCESS;
}

static DCXO_CALI_DIAG_SUBCMD dcxo_cali_mode = DCXO_CALI_DIAG_ENTER_HP_MODE;

/**
 * aic_dcxo_cali_diag_handle - dcxo cali diag handle.
 *  diag cmd:
 *  magic | sn | sys | errno | cmd | subcmd | len | data | tail
 *
 * Parameter:
 * @diag_cmd: receive diag parameter.
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_dcxo_cali_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    bool resp_subcmd = AIC_FALSE;
    int ret, pmic_id, resp;
    diag_rsp_cmd_t rsp;
    u32 actual_flags;
    AMSG_T msg;
    u32 val, tmp;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    pmic_id = (pmic_id >> 16) & 0xFFFF;

    switch (diag_cmd->subcmd) {
    case DCXO_CALI_DIAG_ENTER_LP_MODE:
        if (pmic_id == 0x201) {
            /* switch to sw control */
            val = pmic_readl(0x610);
            val |= BIT(2);
            pmic_writel(0x610, val);
            /* enable 26mbuffer use sw register */
            pmic_writel(0x9a8, 0xf0);
        } else if (pmic_id == 0x202) {
            val = pmic_readl(0x68);
            val |= 0x1;
            pmic_writel(0x68, val);

            val = pmic_readl(0x80);
            val &= ~(0x3ff << 10);
            val |= (0x3ff << 10);
            pmic_writel(0x80, val);
        } else {
            /* do nothing now */
        }

        amsg_fill_msg(&msg, AMOD_CLK_CALI, MSG_DCXO_CALI_DIAG_ENTER_LP_MODE, 0);
        ret = amsg_raw_send_msg(AIC_SYS_CP, &msg);
        AIC_ASSERT(ret == AIC_SUCCESS);
        ret = aos_get_event(dcxo_cali_event, DCXO_CALI_EVENT_LP_SUCCESS, AOS_AND_CLEAR, &actual_flags, 1000);
        AIC_ASSERT(ret == AIC_SUCCESS);

        dcxo_cali_mode = DCXO_CALI_DIAG_ENTER_LP_MODE;
        resp_subcmd = AIC_TRUE;
        break;

    case DCXO_CALI_DIAG_ENTER_HP_MODE:
        if (pmic_id == 0x201) {
            /* switch to sw control */
            val = pmic_readl(0x610);
            val |= BIT(2);
            pmic_writel(0x610, val);
            /* enable 26mbuffer use sw register */
            pmic_writel(0x9a8, 0xf0);
        } else if (pmic_id == 0x202) {
            val = pmic_readl(0x88);
            val &= ~0x3;
            val |= 0x1;
            pmic_writel(0x88, val);

            val = pmic_readl(0x68);
            val &= ~0x1;
            pmic_writel(0x68, val);
        } else {
            /* do nothing now */
        }

        amsg_fill_msg(&msg, AMOD_CLK_CALI, MSG_DCXO_CALI_DIAG_ENTER_HP_MODE, 0);
        ret = amsg_raw_send_msg(AIC_SYS_CP, &msg);
        AIC_ASSERT(ret == AIC_SUCCESS);
        ret = aos_get_event(dcxo_cali_event, DCXO_CALI_EVENT_HP_SUCCESS, AOS_AND_CLEAR, &actual_flags, 1000);
        AIC_ASSERT(ret == AIC_SUCCESS);

        dcxo_cali_mode = DCXO_CALI_DIAG_ENTER_HP_MODE;
        resp_subcmd = AIC_TRUE;
        break;

    case DCXO_CALI_DIAG_WRITE_CT:
        val = *(u32 *)diag_cmd->data;
        if (pmic_id == 0x201) {
            pmic_writel(0x60c, (val & 0x1ff));
            aic_udelay(100);
        }

        if (pmic_id == 0x202) {
            val &= ~0xfffffc00;
            tmp = pmic_readl(0x80);
            if (dcxo_cali_mode == DCXO_CALI_DIAG_ENTER_HP_MODE) {
                tmp &= ~0x3ff;
                tmp |= val;
            } else {
                tmp &= ~(0x3ff << 10);
                tmp |= (val << 10);
            }

            pmic_writel(0x80, tmp);
        }

        resp_subcmd = AIC_TRUE;
        break;

    case DCXO_CALI_DIAG_DIV_DCOEFF_HP_32K:
        val = *(u32 *)diag_cmd->data;
        if (pmic_id == 0x201) {
            pmic_writel(0x6fc, (val & 0xfff));
        }

        if (pmic_id == 0x202) {
            val &= ~0xffff0000;
            tmp = pmic_readl(0x4014);
            tmp &= ~0xffff;
            tmp |= val;
            pmic_writel(0x4014, tmp);
        }
        aic_udelay(100);
        resp_subcmd = AIC_TRUE;
        break;

    case DCXO_CALI_DIAG_DIV_DCOEFF_LP_32K:
        val = *(u32 *)diag_cmd->data;
        if (pmic_id == 0x201) {
            pmic_writel(0x700, (val & 0xfff));
        }

        if (pmic_id == 0x202) {
            val &= ~0xffff0000;
            tmp = pmic_readl(0x4014);
            tmp &= ~(0xffff << 16);
            tmp |= (val << 16);
            pmic_writel(0x4014, tmp);
        }
        aic_udelay(100);
        resp_subcmd = AIC_TRUE;
        break;

    case DCXO_CALI_DIAG_GET_26M_TYPE:
       if (pmic_id == 0x201)
           resp_subcmd = AIC_FALSE;
       else {
           resp = aic_cmdline_get_value_int(cl_key_dcxo);
           resp_subcmd = AIC_TRUE;
           diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_DCXO_CALI, diag_cmd->subcmd,
                     &resp, sizeof(resp));
           diag_cmd_response(&rsp);
           return;
       }
       break;

    default:
        resp_subcmd = AIC_FALSE;
        break;
    }

    if (AIC_TRUE == resp_subcmd) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_DCXO_CALI, diag_cmd->subcmd,
                     NULL, 0);
    } else {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_DCXO_CALI, diag_cmd->subcmd,
                     NULL, 0);
    }
    diag_cmd_response(&rsp);
}

int aic_dcxo_cali_diag_init(void)
{
    AMSG_HANDLER_T receive_handle;
    int ret, pmic_id;
    DCXO_CALI cali;
    u8 remote_sys;
    u32 val, tmp;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    pmic_id = (pmic_id >> 16) & 0xFFFF;

    /* bit7:1 controlled by pmu rtc register 0: controller by hw logic */
    /*val = pmic_readl(REG_PMU_RTC_PMU_SPARE_REG);
    val &= ~BIT_7;
    val |= BIT_6;
    pmic_writel(REG_PMU_RTC_PMU_SPARE_REG, val);
    aic_delay(100);
    val |= BIT_7;
    pmic_writel(REG_PMU_RTC_PMU_SPARE_REG, val);
    aic_delay(100);*/

    /* read dcxo cali info and write them to pmic */
    ret = aic_md_read(MD_ID_DCXOCALI_INFO, sizeof(DCXO_CALI), &cali);
    if (ret != AIC_SUCCESS)
        return ret;

    if (cali.AfcCalFlag) {
        if (pmic_id == 0x201) {
            pmic_writel(0x60c, (cali.afccal.CT & 0x1ff));
            aic_udelay(100);
            pmic_writel(0x6fc, (cali.afccal.HPM_32K & 0xfff));
            aic_udelay(100);
            pmic_writel(0x700, (cali.afccal.LPM_32K & 0xfff));
            aic_udelay(100);
        } else if (pmic_id == 0x202){
            /* read CT and write to register */
            val = cali.afccal.CT;
            tmp = cali.Ct_Lp;
            tmp = (tmp & 0xf) << 16;
            val |= tmp;
            pmic_writel(0x80, val);
            aic_udelay(100);

            /* read DIV_DCOEFF and write to register */
            val = cali.afccal.HPM_32K;
            val |= (cali.afccal.LPM_32K << 16);
            pmic_writel(0x4014, val);
            aic_udelay(100);

            /*val = pmic_readl(0xa4);
            val |= (0x7 << 8);
            pmic_writel(0xa4, val);*/

            val = pmic_readl(0x88);
            val &= ~0x3;
            val |= 1;
            pmic_writel(0x88, val);

            val = pmic_readl(0x8c);
            val &= ~0x7f;
            val |= 0x3f;
            pmic_writel(0x8c, val);
        } else {
            /* impossible, do nothing now */
        }
    }

    diag_register_cmd_handler(DIAG_CMD_DCXO_CALI, aic_dcxo_cali_diag_handle, NULL);

    remote_sys = AIC_SYS_CP;
    receive_handle = _clk_cali_ap_receive_handle;
    ret = amsg_register_mod_handler(remote_sys, AMOD_CLK_CALI, receive_handle, NULL);
    AIC_ASSERT(ret == AIC_SUCCESS);

    dcxo_cali_event = aos_create_event("dce");
    AIC_ASSERT(dcxo_cali_event);

    return AIC_SUCCESS;
}

AIC_DRV_P2_INIT_EXPORT(aic_dcxo_cali_diag_init);

