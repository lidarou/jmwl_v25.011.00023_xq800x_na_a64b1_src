/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __TSC_CFG_H__
#define __TSC_CFG_H__

#include <aic_type.h>

struct aic_tsc_cfg {
    u32 press_cnt;
    u32 trigger_cnt;
    u32 debounce_cnt;
    u32 pre_chg_cnt;
    u32 rotation_angle;
};

/* need set by user */
/* TODO: transfer to ms */
struct aic_tsc_cfg tsc_cfg = {
    .press_cnt      = 0x50, /* 800um (unit:10us) */
    .trigger_cnt    = 0xf,  /* 15ms  (unit:1ms)  */
    .debounce_cnt   = 0x50, /* 800um (unit:10us) */
    .pre_chg_cnt    = 0x50, /* 800um (unit:10us) */
    .rotation_angle = 180,
};

#endif /* __TSC_CFG_H__ */
