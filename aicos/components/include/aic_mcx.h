/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the mcx interface.
 */

#ifndef __AIC_MCX_H__
#define __AIC_MCX_H__

#include <aic_osa.h>
#include <aipc_api.h>

#ifdef CONFIG_MCX_DEBUG
/* MCX debug point type */
#define MCX_DEBUG_NOTIFY_SEND       0xED    /* 237 */
#define MCX_DEBUG_NOTIFY_RECV       0xEE    /* 238 */
#define MCX_DEBUG_ACK_SEND          0xEF    /* 239 */
#define MCX_DEBUG_ACK_RECV          0xF0    /* 240 */
#define MCX_DEBUG_EVENT_RECV        0xF1    /* 241 */
#define MCX_DEBUG_FORCE_NOTIFY      0xF2    /* 242 */
#define MCX_DEBUG_ASSERT            0xF3    /* 243 */
#define MCX_DEBUG_ERASE_ENTER       0xF4    /* 244 */
#define MCX_DEBUG_ERASE_EXIT        0xF5    /* 245 */
#define MCX_DEBUG_WRITE_ENTER       0xF6    /* 246 */
#define MCX_DEBUG_WRITE_EXIT        0xF7    /* 247 */
#define MCX_DEBUG_READ_ENTER        0xF8    /* 248 */
#define MCX_DEBUG_READ_EXIT         0xF9    /* 249 */
#define MCX_DEBUG_LOCK_ENTER        0xFA    /* 250 */
#define MCX_DEBUG_LOCK_EXIT         0xFB    /* 251 */
#define MCX_DEBUG_UNLOCK_ENTER      0xFC    /* 252 */
#define MCX_DEBUG_UNLOCK_EXIT       0xFD    /* 253 */
#define MCX_DEBUG_IS_LOCK_ENTER     0xFE    /* 254 */
#define MCX_DEBUG_IS_LOCK_EXIT      0xFF    /* 255 */

#define MCX_DEBUG_POINT_RECORD(type, event) mcx_debug_point_record(type, event)
void mcx_debug_point_record(u32 type, u32 event);
#else
#define MCX_DEBUG_POINT_RECORD(type, event)
#endif

#ifdef CONFIG_MCX_SUPPORT

#define MCX_WAIT_SYNC     1
#define MCX_WAIT_ASYNC    2

/* MCX event */
#define MCX_SUS_CMD     (0x1)
#define MCX_RES_CMD     (0x2)

#define CP_IN_IDLE      (0x1)
#define CP_IN_CONN      (0x2)

#ifdef CONFIG_SUBSYS_CP

#define __MCX_CODE  __attribute__((section(".mcx_code")))

#define MCX_THREAD_CREATE(a, b, c, d, e, f, g) \
         _mcx_thread_create((a), (b), (c), (d), (e), (f), (g), AOS_AUTO_START)

#define MCX_IS_IN_MCX_STATUS    _mcx_is_in_mcx_status

#define MCX_CP_ENTER_CONN \
                    _mcx_send_notify(AIC_SYS_AP, CP_IN_CONN, MCX_WAIT_SYNC)
#define MCX_CP_EXIT_CONN  \
                    _mcx_send_notify(AIC_SYS_AP, CP_IN_IDLE, MCX_WAIT_SYNC)

#define MCX_CP_NOTIFY_ASSERT_TO_AP _mcx_notify_cp_assert_to_ap

aos_thread_id _mcx_thread_create(char *thread_name,
                                 aos_standard_entry_func entry_func,
                                 void *input, u32 stack_size,
                                 u32 queue_num, char *queue_name,
                                 u32 priority, u32 auto_start);

u32 _mcx_standard_thread_create(aos_thread_id thread_id,
                                char *thread_name,
                                aos_standard_entry_func entry_func,
                                void *input, u32 stack_size,
                                u32 intq_num, char *intq_name,
                                u32 extq_num, char *extq_name,
                                u32 priority, u32 auto_start);

boolean _mcx_is_mcx_enable(void);
void _mcx_notify_cp_assert_to_ap(void);

#elif defined(CONFIG_SUBSYS_AP) /* CONFIG_SUBSYS_CP */

#define MCX_SUSPEND(a) \
{ \
    if (mcx_is_eb()) \
        _mcx_send_notify(AIC_SYS_CP, MCX_SUS_CMD, (a)); \
}
#define MCX_RESUME(a) \
{ \
    if (mcx_is_eb()) \
        _mcx_send_notify(AIC_SYS_CP, MCX_RES_CMD, (a)); \
}
#define MCX_FORCE_RESUME \
{ \
    if (mcx_is_eb()) \
        _mcx_force_notify(AIC_SYS_CP, MCX_RES_CMD); \
}

#define MCX_IS_CP_IN_CONNECT _mcx_is_cp_in_connect

boolean _mcx_is_cp_in_connect(void);
void mcx_eb_set(boolean eb);
boolean mcx_is_eb();
void mcx_ap_force_set_event(void);
void mcx_ap_force_clear_event(void);
#endif /* CONFIG_SUBSYS_CP */

void _mcx_send_notify(u8 sys, u32 event, u32 wait_type);
void _mcx_force_notify(u8 sys, u32 event);
int mcx_init(void);

#else /* CONFIG_MCX_SUPPORT */

#ifdef CONFIG_SUBSYS_CP

#define __MCX_CODE
#define MCX_THREAD_CREATE(a, b, c, d, e, f, g) \
         _mcx_thread_create((a), (b), (c), (d), (e), (f), (g), AOS_AUTO_START)
#define MCX_IS_IN_MCX_STATUS
#define MCX_CP_ENTER_CONNECT
#define MCX_CP_EXIT_CONNECT
#define MCX_CP_NOTIFY_ASSERT_TO_AP()

#define _mcx_thread_create          aos_create_thread
#define _mcx_standard_thread_create aos_create_standard_thread

#elif defined(CONFIG_SUBSYS_AP)

#define MCX_EXIT
#define MCX_ENTER

#endif /* CONFIG_SUBSYS_CP */
#endif /* CONFIG_MCX_SUPPORT */

#endif /* __AIC_MCX_H__ */
