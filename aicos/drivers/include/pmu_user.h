/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines pmu driver api.
 *    The pmu api will be called by pmu core subsys.
 */

#ifndef __PMU_USER_H__
#define __PMU_USER_H__
#include <aic_list.h>
struct pmu_unit;

struct pmu_user {
    struct pmu_unit *punit;
    struct list_head users_list;
    char *user_name;
};

int pmu_enable(struct pmu_user *puser);
int pmu_disable(struct pmu_user *puser);
int pmu_force_disable(struct pmu_user *puser);
int pmu_current_set(struct pmu_user *puser, u32 current_uA, u32 tolerance_uA);
int pmu_current_get(struct pmu_user *puser, u32 *current_uA);
int pmu_voltage_set(struct pmu_user *puser, u32 voltage_uV, u32 tolerance_uV);
int pmu_voltage_get(struct pmu_user *puser, u32 *voltage_uV);
int pmu_is_enabled(struct pmu_user *puser);
int pmu_reset(struct pmu_user *puser);
struct pmu_user *pmu_get(char *user_name, char *unit_name);
int pmu_put(struct pmu_user *puser);

#endif /* __PMU_DRIVER_H__ */
