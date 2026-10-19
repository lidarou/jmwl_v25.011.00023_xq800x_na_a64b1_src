/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines pmu driver kit api.
 *    The pmu kit api will supply pmu driver tools.
 */

#ifndef __PMU_KIT_H__
#define __PMU_KIT_H__

struct pmu_unit;

int pmu_kit_set_voltage_linear(struct pmu_unit *punit, u32 voltage_uV, u32 tolerance_uV);
int pmu_kit_get_voltage_linear(struct pmu_unit *punit, u32 *voltage_uV);
int pmu_kit_set_current_table(struct pmu_unit *punit, u32 current_uA, u32 tolerance_uA);
int pmu_kit_get_current_table(struct pmu_unit *punit, u32 *current_uA);
int pmu_kit_is_enabled(struct pmu_unit *punit);
int pmu_kit_enable(struct pmu_unit *punit);
int pmu_kit_disable(struct pmu_unit *punit);
int pmu_kit_mulit_bit_enable(struct pmu_unit *punit);
int pmu_kit_mulit_bit_disable(struct pmu_unit *punit);
int pmu_io_update(const struct pmu_io *pio, u32 pmu_reg, u32 pmu_msk, u32 pmu_val);

#endif /* __PMU_KIT_H__ */
