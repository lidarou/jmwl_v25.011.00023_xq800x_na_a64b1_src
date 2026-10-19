
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __AIC_KPC_H__
#define __AIC_KPC_H__

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>

#define KPC_KEY_EVENT_PRESS             (1)
#define KPC_KEY_EVENT_RELEASE           (2)
#define KPC_KEY_EVENT_LONGKEY           (4)

#define KPC_KEY_PRESSED_MAX_KEY_NUM     (5) /* include power key*/
typedef struct kpc_key {
    u16 key_value;
    u16 key_event;
}kpc_key_t;

typedef struct kpc_key_array {
    kpc_key_t key[KPC_KEY_PRESSED_MAX_KEY_NUM];/* key[4] is power key */
}kpc_key_array_t;

typedef int32_t (*srv_func_callback)(void *p_param, uint32_t size);

/**
* kpc_read_key - Brief description of function.
* Parameter: [In]: None
*            [Out]: p_key_arr: the point of the key array
*            [Return]: int: the valid key number
* Description: This function is used to register keypad.
*/
int kpc_read_key( kpc_key_array_t * p_key_arr );


int32_t aic_srv_kpc_register(char *client_name, srv_func_callback _srv_callback);


int32_t aic_srv_kpc_unregister(int32_t client_handle);


#endif /* __AIC_KPC_H__ */
