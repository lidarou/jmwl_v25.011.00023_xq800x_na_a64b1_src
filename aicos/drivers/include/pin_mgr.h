/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   Header file of pin managment.
*/
#ifndef __PIN_MGR_H__
#define __PIN_MGR_H__

/* digital pin configuration item for specified board id */
typedef struct dig_pin_cfg_s {
    const u32 (*pin_cfg)[2];
    u32 size;
}dig_pin_cfg_t;

/* pmu pin configuration item for */
typedef struct pmu_pin_cfg_s {
    const u32 (*first_pin_cfg)[2];
    u32 first_size;
    const u32 (*second_pin_cfg)[2];
    u32 second_size;
}pmu_pin_cfg_t;

#endif /* __PIN_MGR_H__ */
