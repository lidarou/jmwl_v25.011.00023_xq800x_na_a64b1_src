/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __KPC_CODES_H__
#define __KPC_CODES_H__

#include <aic_type.h>

typedef enum {
    KEY_0 = 0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_POWER,

    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_ENTER,
    KEY_ESC,
    KEY_BACK,
    KEY_HOME,
    KEY_GREEN,
    KEY_RED,
    KEY_END,
    KEY_DEL,
    KEY_HASH,
    KEY_STAR,

/* TODO: add more keys. */
    KEY_RESERVED,
    KEY_MAX
}kpc_key_e;


#endif /* __KPC_CODES_H__ */
