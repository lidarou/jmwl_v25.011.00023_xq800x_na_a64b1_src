/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log outputer header file.
 */
#ifndef __ALOG_OUTPUTER_H__
#define __ALOG_OUTPUTER_H__
#include <aic_type.h>
#include <aic_io_dev.h>
#include <aic_osa.h>
#include <aipc_brr_inputer.h>
#include <alog_cfg.h>

#ifdef CONFIG_CONNECT_MGR
#include <connmgr_api.h>
#include <aic_list.h>
#endif

#ifdef CONFIG_CONNECT_MGR
//#define CONFIG_ALOG_BUF_BLOCK
#endif

/**
 * ALOG_OUTPUTER - the log outputer define. It contains some brr inpputers
 *                 and a output device.
 * @inputer_cnt: All the brr inputer counter.
 * @dev_type: the io dev type of the output device.
 * @dev_com: the dev com of the output device.
 * @io_dev: the output device.
 * @inputer: the brr inputer array.
 * @mutex:lock the outputer.
 */
typedef struct ALOG_OUTPUTER {
    u8              inputer_cnt;
    u8              dev_type;
    u8              dev_com;
    volatile u8     b_stop;
#ifdef CONFIG_CONNECT_MGR
    aos_thread_id   thread;
    aos_timer_ptr   del_timer;
#else
    AIO_DEV_T       *io_dev;
    AIO_DEV_T       *msg_dev;
#endif
    ABRR_INPUTER_T  **inputer;
    aos_mutex_ptr   mutex;
#ifdef CONFIG_ALOG_BUF_BLOCK
    u16                 list_cnt;
    struct list_head    blk_list;
    aos_mutex_ptr       list_mutex;
#endif
} ALOP_T;

ALOP_T *alop_create(u8 dev_type, u8 com);
void alop_create_inputer(ALOP_T *alop, u8 cnt, const ALBRR_CFG_T *brr_cfg);
void alop_destroy(ALOP_T * alop);
void alop_output_flush_in_assert(ALOP_T *alop);
void alop_output_block_in_assert(ALOP_T *alop, u8 link_id, ABLK_T *blk);

#endif /* __ALOG_OUTPUTER_H__ */

