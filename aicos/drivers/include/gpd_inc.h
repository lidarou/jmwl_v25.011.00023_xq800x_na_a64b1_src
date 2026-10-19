/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines gpd driver api.
 */

#ifndef __GPD_INC_H__
#define __GPD_INC_H__

typedef enum {
    GPD_COUNTER0,
    GPD_COUNTER1,
    GPD_COUNTER2,
    GPD_COUNTER3,
    GPD_COUNTER_MAX,
} GPD_INDEX;

typedef enum {
    GPD_POSEDGE_TYPE = 0,
    GPD_NEGEDGE_TYPE,
    GPD_BOTHEDGE_TYPE
} GPD_EDGE_TYPE;

void aic_gpd_update(GPD_INDEX index);
void aic_gpd_mode_cfg(GPD_INDEX index, GPD_EDGE_TYPE mode);
void aic_gpd_cnt_cfg(GPD_INDEX index, u32 cnt);
u32 aic_gpd_get_current_cnt(GPD_INDEX index);
u32 aic_gpd_get_cnt(GPD_INDEX index);
u32 aic_gpd_get_cycle(GPD_INDEX index);

#endif /* __GPD_INC_H__ */

