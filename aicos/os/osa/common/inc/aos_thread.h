/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AOS Thread Interface.
 */

#ifndef __AOS_THREAD_H__
#define __AOS_THREAD_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#include <aic_osa.h>

/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/
/* thread control block*/

#define MAX_Q_NAME_LEN  19
#define Q_NAME_SIZE     (MAX_Q_NAME_LEN + 1)

typedef struct aos_thread_desc_st {
#ifdef CONFIG_RTOS_THREADX
    TX_THREAD   tcb_ptr;
    TX_QUEUE    *int_qcb;
    TX_QUEUE    *ext_qcb;
#elif defined (CONFIG_RTOS_LITEOS_M)
    u32         los_task;
    u32         intq_id;
    u32         extq_id;
    u8 *        intq_ptr;
    u8 *        extq_ptr;
#endif
} aos_thread_desc;

#define INVALID_QUEUE_ID            0xffffffff
#define INVALID_TASK_ID             0xffffffff

/**---------------------------------------------------------------------------*/
/**                          function prototype                               */
/**---------------------------------------------------------------------------*/

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __AOS_THREAD_H__ */

