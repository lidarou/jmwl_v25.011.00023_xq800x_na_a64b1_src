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

#ifndef __PMU_DRIVER_H__
#define __PMU_DRIVER_H__
#include <aic_osa.h>
#include <aic_list.h>

//#define PMU_DEBUG
#ifdef PMU_DEBUG
#define pmu_dbg alog_warn
#else
#define pmu_dbg(fmt, ...)
#endif


struct pmu_user;
struct pmu_unit;

struct pmu_ops {
    int (*set_voltage)(struct pmu_unit *punit, u32 voltage_uV, u32 tolerance_uV);
    int (*get_voltage)(struct pmu_unit *punit, u32 *voltage_uV);
    int (*set_current)(struct pmu_unit *punit, u32 current_uA, u32 tolerance_uA);
    int (*get_current)(struct pmu_unit *punit, u32 *current_uA);
    int (*enable)(struct pmu_unit *punit);
    int (*disable)(struct pmu_unit *punit);
    int (*is_enabled)(struct pmu_unit *punit);
};

struct pmu_info {
    char *unit_name;
    char *parent_name;
    const uint32_t *current_table;
    uint32_t current_step_num;
    uint32_t current_reg;
    uint32_t current_msk;
    uint32_t current_min_uA;
    uint32_t current_max_uA;
    uint32_t voltage_reg;
    uint32_t voltage_msk;
    uint32_t enable_reg;
    uint32_t enable_msk;
    uint32_t cfg_update_reg;
    uint32_t cfg_update_msk;
    uint32_t voltage_min_uV;
    uint32_t voltage_max_uV;
    uint32_t voltage_step_uV;
    uint32_t scaling_boot_voltage;
    int scaling_time_us;
    int pd_time_us;
    bool enalbe_bit_inverted;
    bool always_on;
    bool boot_on;
    bool bypass;
    const struct pmu_ops *pops;
};

struct pmu_io {
    int (*pmu_read)(u32 reg, u32 *val);
    int (*pmu_write)(u32 reg, u32 val);
};

struct pmu_unit {
    char *unit_name;
    struct pmu_io *pmu_io;
    struct pmu_info *pinfo;
    struct pmu_user *parent_unit;
    struct list_head child_list_head;
    struct list_head child;
    struct list_head unit_list; /*current pmu unit*/
    aos_mutex_ptr unit_mutex;
    int enable_depth;
};
void pmu_show_unit_list(void);
void pmu_show_user_list(void);
int pmu_register(struct pmu_info * pinfo, struct pmu_io * pio);
void pmu_unregister(struct pmu_unit* punit);
#endif /* __PMU_DRIVER_H__ */
