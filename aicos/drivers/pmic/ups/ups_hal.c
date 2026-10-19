/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of ups module, it is ups
 *    common hal driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x033809
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_at_process_common.h>
#include <aic_at_resp_handle.h>
#include <aic_bitmap.h>
#include <aic_cmd_api.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_type.h>
#include <core_op.h>
#include <irq.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <pmic_irq.h>
#include <pmic_rco32k_cali.h>
#include <stdlib.h>
#include <sysoff.h>

#include "ups_interface.h"

/*
 * ups_enter - enter ups mode.
 * Before enter ups mode, it needs to set rc32K calibration
 * configuration to make sure that the ups rc32k is precisely.
 */
void ups_enter(void)
{
    aic_rco32k_hw_cali_interval_for_ups(60);

    gx_ups_phy_mode_enter();
}

/*
 * gx_ups_cnt_clear - clear ups cnt.
 * It will recount from 0.
 */
void gx_ups_cnt_clear(void)
{
    gx_ups_phy_cnt_clr();
}

/*
 * gx_ups_wakeup_time - set ups wake up time.
 * It will clean ups count, and set wake up time,
 * the time unit is second.
 */
void gx_ups_wakeup_time(u32 sec)
{
    u32 cnt = sec * GX_UPS_CLK;

    gx_ups_cnt_clear();

    gx_ups_phy_set_comp(cnt);
}

/*
 * gx_ups_start_count - start ups count.
 */
void gx_ups_start_count(void)
{
    gx_ups_phy_start_count();
}

void gx_ups_set_comp(u32 cnt)
{
    gx_ups_phy_set_comp(cnt);
}

u32 gx_ups_get_cnt(void)
{
    return gx_ups_phy_get_cnt();
}

u32 gx_ups_int_status(void)
{
    return gx_ups_phy_get_int_status();
}

void gx_ups_set_sw_reg(u32 val)
{
    gx_ups_phy_set_sw_reg(val);
}

u32 gx_ups_get_sw_reg(void)
{
    return gx_ups_phy_get_sw_reg();
}

void gx_ups_cfg_period_mode(void)
{
    gx_ups_phy_set_period();
}

/**
 * gx_ups_at_get_cnt Read current ups count by at command
 * AT+UPS=?
 */
static u32 gx_ups_at_get_cnt(AT_REQ_RESP *at_req_res_ptr)
{
    ulong line_pointers[5];
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u32 cnt = gx_ups_get_cnt();

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "%d", cnt);
    line_pointers[0] = (ulong)&result_line[0];

    return aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
}

/**
 * gx_ups_at_set_cnt Set ups sleep count by at command
 * AT+UPS=<sec>
 * sec: the ups sleep count(sec)
 */
static u32 gx_ups_at_set_cnt(AT_REQ_RESP *at_req_res_ptr)
{
    ulong line_pointers[5];
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong *paras_arr;
    u16 paras_count = 0;
    u32 error_code, sec;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+UPS=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        error_code = AT_ERROR_INVALID_PARAS;
        goto PARA_ERROR;
    }

    sec = strtoul((char *)paras_arr[0], NULL, 10);

    gx_ups_wakeup_time(sec);
    ups_enter();

    strcpy(result_line, "SUCCESS");
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+CME ERROR: %d", error_code);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    return AIC_SUCCESS;
}

AT_CMD_HANDLE ups_at_handles[] = {
    {"AT+UPS", gx_ups_at_get_cnt, AIC_NULL, gx_ups_at_set_cnt, AIC_NULL},
    {AIC_NULL},
};

void gx_ups_to_rtc_sync(void)
{
    gx_ups_phy_to_rtc();
}

void gx_rtc_to_ups_sync(void *ptr)
{
    gx_ups_phy_from_rtc();

    gx_ups_phy_cnt_clr();
}

void gx_ups_irq_handler(void *param)
{
    gx_ups_phy_int_clr();
}

int gx_ups_init(void)
{
    register_pmic_irq(PMIC_IRQ_UPS_CNT, gx_ups_irq_handler, AIC_NULL);

    sysoff_register_hook(gx_rtc_to_ups_sync, AIC_NULL);

    gx_ups_phy_init();

    aic_add_at_cmd_handle(ups_at_handles);

#ifdef CONFIG_DEF_UPS_MODE
    ups_enter();
#else
    /* exit ups mode by default */
    gx_ups_phy_mode_exit();
#endif

    /* set ups sw reg as 0, it will be set when sysoff */
    gx_ups_phy_set_sw_reg(0);

    return 0;
}

AIC_DRV_L2_INIT_EXPORT(gx_ups_init);

