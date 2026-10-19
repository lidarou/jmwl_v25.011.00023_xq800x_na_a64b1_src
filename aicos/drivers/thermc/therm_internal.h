/*****************************************************************************/
/*                                                                           */
/*         Copyright 2022 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of thermal controller module.
 *   It includes all the thermal controller phy function api.
*/
#ifndef __THERMC_H__
#define __THERMC_H__
#include <aic_osa.h>
#include <aic_bitmap.h>

#define THERMC_ADC_CLK                          4000000     //4M
#define THERMC_DEGREE_KELVIN_VAL                600         //27C degree kelvin 273+27 = 300��config 600
#define THERMC_ADC_SAMPLE_CYCLE                 2           //us
#define THERMC_AUTO_MONITOR_CYCLE               100         //ms
#define THERMC_OVER_HEAT_IRQ_THRES              692         //(692/2)-273for cv case test
#define THERMC_OVER_HEAT_RST_THRES              769         //for cv case test
#define THERMC_UNDER_CODE_IRQ_THRES             461         //(-42.5)for cv case test
#define THERMC_SIMULATE_ADC_NUMS                300
#define THERMC_SIMULATE_ADC_FILL_CYCLE          100         //ms
#define THERMC_SIMULATE_ADC_FILL_CYCLE_CALIB    10          //ms
#define THERMC_RETYR_MAX                        5
#define THERMC_CASE_RESULT_CHECK_CYCLE          100         //ms
#define THERMC_NORMAL_TEMP_ADC                  4000
//#define THERMC_DEBUG
//#define THERMC_SOC_IRQ

struct thermal_sensor_ops {
    int (*init)(void);
    int (*start)(void);
    int (*stop)(void);
    u32 (*get_factor)(void);
};

typedef enum {
    AIC_THERMC_IRQ_CALIB_DONE     = BIT_0,
    AIC_THERMC_IRQ_DET_DONE       = BIT_1,
    AIC_THERMC_IRQ_AUTO_DET_ALRM0 = BIT_2,
    AIC_THERMC_IRQ_AUTO_DET_ALRM1 = BIT_3,
    AIC_THERMC_IRQ_AUTO_DET_ALRM2 = BIT_4,
    AIC_THERMC_IRQ_ALL = BIT_0 | BIT_1 | BIT_2 | BIT_3 | BIT_4
} AIC_THERMC_IRQ_MSK;

typedef enum {
    AIC_THERMC_SIMULATE_ADC_CALI            = 0,
    AIC_THERMC_SIMULATE_ADC_DET             = 1,
    AIC_THERMC_SIMULATE_ADC_AUTO_OVERHAET   = 2,
    AIC_THERMC_SIMULATE_ADC_AUTO_UNDERCOLD  = 3
} AIC_THERMC_SIMULATOR_ADC_TYEP;

typedef enum {
    AIC_THERMC_MODE_CLEAR = 0,
    AIC_THERMC_AUTO_DET_MODE = BIT(0),
    AIC_THERMC_SIG_DET_MODE  = BIT(1),
    AIC_THERMC_CALIB_MODE    = BIT(2)
} AIC_THERMC_WORK_MODE;

/*thermc interface*/
int aic_thermc_module_enable(void);
int thermc_all_reg_reset_val(void);
void aic_calib_env_temp_set(u32 env_temp);
void aic_thermc_irq_init(void);
void aic_thermc_irq_enabled(AIC_THERMC_IRQ_MSK thermc_irq_msk, bool enabled);
void aic_thermc_work_mode_select(AIC_THERMC_WORK_MODE mode);
int aic_thermc_adc_simulate_result(void);
u16 aic_thermc_cali_factor_get_via_int(void);
void aic_thermc_irq_clear(AIC_THERMC_IRQ_MSK thermc_irq_msk);
int aic_thermc_case_result_check(void);
void aic_thermc_heat_thres_set(u16 irq_val, u16 reset_val);
void aic_thermc_auto_det_cycle_set(u32 auto_det_cycle_ms);
u16 aic_thermc_sig_det_temp_get(void);
void aic_thermc_adc_sample_cycle(u32 sample_rate_us);
void aic_thermc_cali_factor_set(u16 cali_factor_from_nv);
u16 aic_thermc_sig_det_temp_get_via_int(void);
void aic_thermc_cold_thres_set(u16 cold_thres_val);
void aic_tsensor_register(struct thermal_sensor_ops *tsensor_ops);

#endif /* __THERMC_H__ */
