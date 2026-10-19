/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __KPC_CFG_H__
#define __KPC_CFG_H__

#include <aic_type.h>
#include <irq_chip.h>

#include "kpc_codes.h"

/* need set by user */
#define KPC_KEY_ROW_BITS        (0x1F)  /* config key-in by bits */
#define KPC_KEY_COL_BITS        (0x0F)  /* config key-out by bits */

#define NUMBER_OF_BITS(x)       ( ((x>>0)&0x01) + ((x>>1)&0x01) + ((x>>2)&0x01) + ((x>>3)&0x01) + \
                                  ((x>>4)&0x01) + ((x>>5)&0x01) + ((x>>6)&0x01) + ((x>>7)&0x01) )

#define KPC_KEY_ROW_NUM         NUMBER_OF_BITS(KPC_KEY_ROW_BITS)
#define KPC_KEY_COL_NUM         NUMBER_OF_BITS(KPC_KEY_COL_BITS)

struct kpc_config {
    u32 rows;
    u32 cols;

    u32 press_debounce_cnt;
    u32 release_debounce_cnt;
    u32 long_press_cnt;
    u32 auto_slp_cnt;

    /* whether support ghost key */
    boolean need_ghost_filter;
    boolean auto_sleep;
    boolean longkey_en;
    boolean level;

    u32 irq;
};

/* need set by user */
struct kpc_config kpc_config = {
    .rows = KPC_KEY_ROW_BITS,
    .cols = KPC_KEY_COL_BITS,

    .press_debounce_cnt = 0xf,
    .release_debounce_cnt = 0xf,
    .long_press_cnt = 0x400,
    .auto_slp_cnt = 0x80000,

    .need_ghost_filter = FALSE,
    .auto_sleep = TRUE,
    .longkey_en = FALSE,
    .level = 0,

    .irq = IRQ_KPD,
};

kpc_key_e kpc_key_map[KPC_KEY_ROW_NUM][KPC_KEY_COL_NUM] =
{
    {KEY_5,       KEY_1,      KEY_LEFT,      KEY_3},
    {KEY_2,       KEY_4,      KEY_RIGHT,     KEY_6},
    {KEY_DOWN,    KEY_GREEN,  KEY_ENTER,     KEY_BACK},
    {KEY_8,       KEY_STAR,   KEY_HOME,      KEY_HASH},
    {KEY_0,       KEY_7,      KEY_UP,        KEY_9},
};

#endif /* __KPC_CFG_H__ */
