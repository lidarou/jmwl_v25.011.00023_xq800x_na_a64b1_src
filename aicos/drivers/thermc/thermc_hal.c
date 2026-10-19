/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of thermal HAL module
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0c488b
#define LOG_MODULE    AMOD_DRV

#include <aic_io.h>
#include <aic_osa.h>
#include <aic_at_process_common.h>
#include <aic_cmdline.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include <irq_chip.h>
#include "./therm_internal.h"
#include <slp_inc.h>
#include <timer_inc.h>

u16 thermc_cali_factor;

struct thermal_sensor_ops *cv_tsensor_ops = NULL;
aos_timer_ptr   therm_monitor_timer;
#define THERM_MON_PERIOD_TIME   10000

#ifdef THERMC_AUTO
static int thermc_calib_mode()
{
    int ret = AIC_EIO;
    int counter;

    aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);

    aic_thermc_module_enable();

    /* 2.set adc sample cycle us*/
    aic_thermc_adc_sample_cycle(THERMC_ADC_SAMPLE_CYCLE);

    /* 1.environment degree kelvin value set*/
    aic_calib_env_temp_set(THERMC_DEGREE_KELVIN_VAL);

    /* 3.thermc irq handler init*/
    aic_thermc_irq_init();

    /* 4.enable therm calibrate mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_CALIB_DONE, true);

    if (cv_tsensor_ops->start)
        cv_tsensor_ops->start();

    counter = 100;

    aic_udelay(1000);
    /* 5.therm_calib mode select */
    aic_thermc_work_mode_select(AIC_THERMC_CALIB_MODE);
    aic_mdelay(10);

    while (counter--) {
        thermc_cali_factor = aic_thermc_cali_factor_get_via_int();
        if (thermc_cali_factor) {
            ret = 0;
            break;
        }
        aic_mdelay(10);
        aicx_warn(0x009d99, "", "calibrating ...!\n");
    }

    if (ret) {
        aicx_warn(0x0be7cf, "", "calibrate fail!\n");
        while(1);
    }

    /* 7.disable therm calibrate mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_CALIB_DONE, false);

    if (cv_tsensor_ops->stop)
        cv_tsensor_ops->stop();
    else
        aicx_warn(0x163225, "", "no need to stop thermal sensor capture.\n");

    aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);

    aic_thermc_irq_clear(AIC_THERMC_IRQ_ALL);

    return ret;
}

static int thermc_sig_det_mode()
{
    int ret;
    u16 det_temp = 0;
    int counter;

    ret = AIC_EIO;
    if (!thermc_cali_factor) {
        ret = AIC_EIO;
        aicx_warn(0x1515b1, "", "none thermc cali factor get from priv case!");
        goto out;
    }

    /* 1.load thermc cali factor from nv, we load pre case value in this cv case.*/
    aic_thermc_cali_factor_set(thermc_cali_factor);

    /* 2.set adc sample cycle us*/
    aic_thermc_adc_sample_cycle(THERMC_ADC_SAMPLE_CYCLE);

    /* 3.thermc irq handler init*/
    aic_thermc_irq_init();

    /* 4.enable therm sig mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_DET_DONE, true);

    aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);
    /* 5.therm det mode select */
    aic_thermc_work_mode_select(AIC_THERMC_SIG_DET_MODE);

    cv_tsensor_ops->start();
    counter = 30;

    while (counter--) {
        aic_mdelay(10);
        det_temp = aic_thermc_sig_det_temp_get_via_int();
        aic_mdelay(2000);
        aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);
        /* 5.therm det mode select */
        aic_thermc_work_mode_select(AIC_THERMC_SIG_DET_MODE);
    }

    det_temp = aic_thermc_sig_det_temp_get_via_int();

    if (det_temp)
        ret = 0;

    /* 7.disable thermc det mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_DET_DONE, false);

    if (cv_tsensor_ops->stop)
        cv_tsensor_ops->stop();
    else
        aicx_warn(0x163225, "", "no need to stop thermal sensor capture.\n");

out:
    aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);

    aic_thermc_irq_clear(AIC_THERMC_IRQ_ALL);

    return ret;
}

static int thermc_auto_det_overheat_irq()
{
    int ret;
    u32 temp;

    if (!thermc_cali_factor) {
        ret = AIC_EIO;
        aicx_warn(0x1515b1, "", "none thermc cali factor get from priv case!");
        goto out;
    }

    /* 1.load thermc cali factor from nv, we load pre case result cali_factor value in this cv case.*/
    aic_thermc_cali_factor_set(thermc_cali_factor);

    /* 2.set adc sample cycle us*/
    aic_thermc_adc_sample_cycle(THERMC_ADC_SAMPLE_CYCLE);

    /* 3.set thermc auto detect monitor cycle*/
    aic_thermc_auto_det_cycle_set(THERMC_AUTO_MONITOR_CYCLE);

    /* 4.set thermc over heat irq&rset thres value*/
    aic_thermc_heat_thres_set(THERMC_OVER_HEAT_IRQ_THRES, THERMC_OVER_HEAT_RST_THRES);

    /* 5.thermc irq handler init*/
    aic_thermc_irq_init();

    /* 6.enable thermc auto mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_AUTO_DET_ALRM0, true);

    aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);
    /* 7.therm auto work mode select */
    aic_thermc_work_mode_select(AIC_THERMC_AUTO_DET_MODE);

    /* 8.start loopback fill adc simulate data task */

    aic_thermc_irq_enabled(AIC_THERMC_IRQ_ALL, true);
    aic_thermc_heat_thres_set(THERMC_OVER_HEAT_IRQ_THRES, THERMC_OVER_HEAT_RST_THRES);
    aic_thermc_cold_thres_set(THERMC_UNDER_CODE_IRQ_THRES);

    while (1) {
        temp = aic_thermc_sig_det_temp_get();

        temp = temp/2 - 273;

        aicx_warn(0x0bb187, "d", "current temp:%d\n",  temp);
        aic_mdelay(2000);
    }

//    aic_thermc_loopback_simulate_start(AIC_THERMC_SIMULATE_ADC_AUTO_OVERHAET);

//    ret = aic_thermc_result_check();

    /* 9.disable thermc calibrate mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_AUTO_DET_ALRM0, false);

    /* 10.stop loopback fill adc simulate data task */
//    aic_thermc_loopback_simulate_stop();

out:
    aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);

    aic_thermc_irq_clear(AIC_THERMC_IRQ_ALL);
    return ret;
}

static int thermc_auto_det_overheat_rst()
{
    int ret;

    if (!thermc_cali_factor) {
        ret = AIC_EIO;
        aicx_warn(0x1515b1, "", "none thermc cali factor get from priv case!");
        goto out;
    }

    /* 1.load thermc cali factor from nv, we load pre case result cali_factor value in this cv case.*/
    aic_thermc_cali_factor_set(thermc_cali_factor);

    /* 2.set adc sample cycle us*/
    aic_thermc_adc_sample_cycle(THERMC_ADC_SAMPLE_CYCLE);

    /* 3.set thermc auto detect monitor cycle*/
    aic_thermc_auto_det_cycle_set(THERMC_AUTO_MONITOR_CYCLE);

    /* 4.set thermc over heat irq&rset thres value*/
    aic_thermc_heat_thres_set(THERMC_OVER_HEAT_IRQ_THRES, THERMC_OVER_HEAT_RST_THRES);

    /* 5.thermc irq handler init*/
    aic_thermc_irq_init();

    /* 6.enable thermc auto mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_AUTO_DET_ALRM0 | AIC_THERMC_IRQ_AUTO_DET_ALRM1, true);

    /* 7.therm auto work mode select */
    aic_thermc_work_mode_select(AIC_THERMC_AUTO_DET_MODE);

    /* 8.start loopback fill adc simulate data task */
//    aic_thermc_loopback_simulate_start(AIC_THERMC_SIMULATE_ADC_AUTO_OVERHAET);

//    ret = aic_thermc_result_check();

    /* 9.disable thermc calibrate mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_AUTO_DET_ALRM0 | AIC_THERMC_IRQ_AUTO_DET_ALRM1, false);

    /* 10.stop loopback fill adc simulate data task */
///    aic_thermc_loopback_simulate_stop();

out:
    aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);

    aic_thermc_irq_clear(AIC_THERMC_IRQ_ALL);
    return ret;
}

static int thermc_auto_det_undercold_irq()
{
    int ret;

    if (!thermc_cali_factor) {
        ret = AIC_EIO;
        aicx_warn(0x1515b1, "", "none thermc cali factor get from priv case!");
        goto out;
    }

    /* 1.load thermc cali factor from nv, we load pre case result cali_factor value in this cv case.*/
    aic_thermc_cali_factor_set(thermc_cali_factor);

    /* 2.set adc sample cycle us*/
    aic_thermc_adc_sample_cycle(THERMC_ADC_SAMPLE_CYCLE);

    /* 3.set thermc auto detect monitor cycle*/
    aic_thermc_auto_det_cycle_set(THERMC_AUTO_MONITOR_CYCLE);

    /* 4.set thermc undercold irq thres value*/
    aic_thermc_cold_thres_set(THERMC_UNDER_CODE_IRQ_THRES);

    /* 5.thermc irq handler init*/
    aic_thermc_irq_init();

    /* 6.enable therm auto mode irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_AUTO_DET_ALRM2, true);

    /* 7.therm auto work mode select */
    aic_thermc_work_mode_select(AIC_THERMC_AUTO_DET_MODE);

    /* 8.start loopback fill adc simulate data task */
//    aic_thermc_loopback_simulate_start(AIC_THERMC_SIMULATE_ADC_AUTO_UNDERCOLD);

//    ret = aic_thermc_result_check();

    /* 9.disable therm auto mode undercold irq*/
    aic_thermc_irq_enabled(AIC_THERMC_IRQ_AUTO_DET_ALRM2, false);

//    /* 10.stop loopback fill adc simulate data task */
//    aic_thermc_loopback_simulate_stop();

out:
    aic_thermc_work_mode_select(AIC_THERMC_MODE_CLEAR);
    aic_thermc_irq_clear(AIC_THERMC_IRQ_ALL);
    return ret;
}
#endif
void aic_tsensor_register(struct thermal_sensor_ops *tsensor_ops)
{
    cv_tsensor_ops = tsensor_ops;
}

//AIC_DRV_P1_INIT_EXPORT(aic_thermc_init);

void thermal_monitor(void *input)
{
    u32 val;
    while(1) {
        val = AIC_READ32(0x41120014);
        aicx_warn(0x0ad3cc, "ddd", "current therm val:%d Temp:%d.%d\n", val, (val/2 -273),val%2?5:0);
        aos_sleep(5000);
    }
}

int get_cur_temp(void)
{
    u32 val;

    val = AIC_READ32(0x41120014);

    return (val/2 -273);
}

void thermal_auto_monitor(void *param)
{
    u32 val;

    val = AIC_READ32(0x41120014);
    aicx_warn(0x0ad3cc, "ddd", "current therm val:%d Temp:%d.%d\n", val, (val/2 -273), val%2?5:0);
}

int therm_hal_init(void)
{
    int ret = 0;

    ret = cv_tsensor_ops->init();

    therm_monitor_timer = aos_create_timer("therm_monitor_timer",
                                         thermal_auto_monitor, NULL,
                                         THERM_MON_PERIOD_TIME,
                                         AOS_NO_ACTIVATE, AOS_BE_PERIOD);
    if (!therm_monitor_timer) {
        aicx_warn(0x1dd5d8, "", "therm_monitor_timer create fail!\n");
        ret = AIC_EIO;
    }

    return ret;
}
AIC_COMP_L2_INIT_EXPORT(therm_hal_init);

#if defined (CONFIG_DRIVER_THERMC) && (CONFIG_SUBSYS_AP)
static u32 at_thermc_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^THM");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "ON,PERIOD_CYCLE_S/OFF.");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_thermc_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    ulong rsp[1];
    int ret = 0;
    u16 count;
    u32 period_cycle = 0;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^THM=", &count);
    if (!paras_arr || count > 2) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (!strcmp((char *)paras_arr[0],"ON") || !strcmp((char *)paras_arr[0],"on")
        || !strcmp((char *)paras_arr[0],"On")) {
        period_cycle = atoi((u8 *)paras_arr[1]);
        if (!period_cycle) {
            ret = AIC_ENXIO;
        } else {
            period_cycle = 1000 * period_cycle;
            aos_change_timer(therm_monitor_timer, period_cycle, AOS_BE_PERIOD);
        }
    } else if (!strcmp((char *)paras_arr[0],"OFF") || !strcmp((char *)paras_arr[0],"off")
        || !strcmp((char *)paras_arr[0],"OFf")) {
         aos_deactivate_timer(therm_monitor_timer);
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

AT_CMD_HANDLE thermc_at_handles[] = {
    {
        "AT^THM",
        at_thermc_test_cmd,
        AIC_NULL,
        at_thermc_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};


int thermc_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&thermc_at_handles[0]);

    return 0;
}
AIC_COMP_L2_INIT_EXPORT(thermc_at_cmd_init);
#endif
