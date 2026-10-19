/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines pmic pmu driver api.
 */

#ifndef __PMIC_PMU_H__
#define __PMIC_PMU_H__

/**
 *  DESCRIPTION
 *
 *  pmic pmu units for current subsys, pmu name use BB_name.
 *
 *   pmu_signal      BB_name
 *   BUCK0           VDDCORE
 *   BUCK1           VDDMP1V88
 *   BUCK2           VDDRFPA
 *   LDO1            AVDDRF1V8
 *   LDO2            AVDD1V8
 *   LDO3            VDDCAMIO1V8
 *   LDO4            AVDDRF1V2
 *   LDO5            DVDD3V
 *   LDO6            AVDDRF2V8
 *   LDO7            VDDBK3V3
 *   LDO8            AVDDCAM2V8
 *   LDO9            VDDSIM0
 *   LDO10           VDDSIM1
 *   LDO11           VDDUSB3V1
 *   LDO12           VDDSD3V
 *   LDO13           VDDTCXO
 *   LDO14           VDDMIC
 */

int pmu_enable(char *pmu_name);
int pmu_disable(char *pmu_name);
int pmu_voltage_set(char *pmu_name, u32 voltage_mV);
int pmu_voltage_get(char *pmu_name, u32 *voltage_mV);
int pmu_is_enabled(char *pmu_name);
#endif /* __PMIC_PMU_H__ */
