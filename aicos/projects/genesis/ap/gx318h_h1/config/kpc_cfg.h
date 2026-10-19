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
#define KPC_KEY_ROW_BITS        (0x3D)  /* config key-in by bits */ /* KEYIN0,KEYIN2,KEYIN3,KEYIN4,KEYIN5 */
#define KPC_KEY_COL_BITS        (0x0F)  /* config key-out by bits */ /* KEYOUT0,KEYOUT1,KEYOUT2,KEYOUT3 */

#define NUMBER_OF_BITS(x)       ( ((x>>0)&0x01) + ((x>>1)&0x01) + ((x>>2)&0x01) + ((x>>3)&0x01) + \
                                  ((x>>4)&0x01) + ((x>>5)&0x01) + ((x>>6)&0x01) + ((x>>7)&0x01) )

#define KPC_KEY_ROW_NUM         (6)//NUMBER_OF_BITS(KPC_KEY_ROW_BITS)
#define KPC_KEY_COL_NUM         (4)//NUMBER_OF_BITS(KPC_KEY_COL_BITS)

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

    .press_debounce_cnt = 0x1f,
    .release_debounce_cnt = 0x1f,
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
    {KEY_RIGHT,     KEY_UP,        KEY_LEFT,      KEY_DOWN},
    {KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED,  KEY_RESERVED},
    {KEY_DEL,       KEY_ESC,       KEY_ENTER,     KEY_HOME},
    {KEY_3,         KEY_2,         KEY_1,         KEY_6},
    {KEY_5,         KEY_4,         KEY_9,         KEY_8},
    {KEY_HASH,      KEY_0,         KEY_STAR,      KEY_7},
};

/* TODO: change to transpose matix with 4 row 3 col
kpc_key_e kpc_key_value[3][4] =
{
    {KEY_ESC,         KEY_UP,         KEY_BACK,          KEY_DEL},
    {KEY_LEFT,        KEY_ENTER,      KEY_RIGHT,         KEY_HOME},
    {KEY_GREEN,       KEY_DOWN,       KEY_RED,           KEY_END},
};
*/

#endif /* __KPC_CFG_H__ */
