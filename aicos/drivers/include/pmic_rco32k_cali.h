/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

#ifndef __PMIC_RCO32K_CALI__
#define __PMIC_RCO32K_CALI__

#ifdef CONFIG_SUBSYS_AP
int aic_rco32k_sw_cali_for_ups(void);
int aic_rco32k_hw_cali_interval_for_ups(u32 interval);
#endif

#define RCO32K_CALI_ONGOING (0)
#define RCO32K_CALI_SUCCESS (1)
#define RCO32K_CALI_TIMEOUT (2)

typedef void (*aic_rco32k_sw_cali_cb)(u8 prepare);
int aic_register_rco32k_sw_cali_cb(aic_rco32k_sw_cali_cb cb);
void aic_unregister_rco32k_sw_cali_cb(void);

#endif /* __PMIC_RCO32K_CALI__ */

