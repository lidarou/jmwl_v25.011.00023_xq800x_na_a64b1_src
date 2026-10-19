/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The aipc bearer input source file.
 *    use the apis of this file can listen for data received
 *    by all marked links of a BRR.
 */
#ifndef __AIPC_BRR_INPUTER_H__
#define __AIPC_BRR_INPUTER_H__
#include <aipc_api.h>
#include <aic_osa.h>

/**
 * CONFIG_INPUTER_SUPPORT_OUTER
 * Enable it will create a output task to to process the blk
 * That reiveied from the inputer brr.
 */
#define CONFIG_INPUTER_SUPPORT_OUTER    0

#define MARKED_ALL_LINK     0xffffffff
typedef void (*blk_handler_t)(u8 sys, u8 brr, u8 link_id, ABLK_T *blk, void *p);

#define BRR_INPUTER_STACK_SIZE  1024
#define BRR_INPUTER_NAME_LEN    10

typedef struct ABRR_OUTER {
    aos_semaphore_ptr sema;
    aos_thread_id   thread;
    ABLK_T          *blk_q;
    u16             q_wt;
    u16             q_rd;
    u16             q_cnt;
    u16             buf_wt;
    u16             buf_rd;
    u16             buf_size;
    u32             lost_cnt;
    u8              *buf;
    char            name[BRR_INPUTER_NAME_LEN];
} ABRR_OUTER_T;

typedef struct ABRR_INPUTER {
    u8              sys;
    u8              brr_id;
    volatile u8     b_stop;
    ABRR_HANDLE     brr;
    u32             link_mark;
    aos_thread_id   thread;
    blk_handler_t  blk_handler;
    void            *data;
    char            name[BRR_INPUTER_NAME_LEN];
#if IS_CONF_ENABLED(CONFIG_INPUTER_SUPPORT_OUTER)
    ABRR_OUTER_T    *outer;
#endif
} ABRR_INPUTER_T;

/**
 * abrr_inputer_create - create a abrr inputer with specified parameters.
 *
 * @sys: the remote system that communicates with the local system.
 * @brr_id: the bearer id.
 * @marked_link: Which links in this brr are to be listened on.
 * @priority, the priority of the inputer listen thread.
 * @outer_buf_size if > 0 means will create a output thread to call blk handler.
 * @handler: after reciver a block, will call the block handler, In the block
 *           handler can be block, but ss soon as the function returns,
 *           the block is freed and cannot be used again.
 * @p: the data pointer of callback handler function.
 */
ABRR_INPUTER_T *abrr_inputer_create(u8 sys, u8 brr_id, u32 marked_link,
                                    u32 outer_buf_size, u32 outer_q_cnt,
                                    u32 priority, blk_handler_t handler,
                                    void *p);

/**
 * abrr_inputer_destroy - destroy the abrr inputer.
 *
 * @inputer: the handle of abrr inputer.
 */
void abrr_inputer_destroy(ABRR_INPUTER_T *inputer);

/**
 * abrr_inputer_start - start the inuter listen therad.
 *
 * @inputer: the handle of abrr inputer.
 */
void abrr_inputer_start(ABRR_INPUTER_T *inputer);

#endif /* __ALOG_OUTPUTER_H__ */

