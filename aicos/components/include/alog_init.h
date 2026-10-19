/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log mudule init header file.
 */
#ifndef __ALOG_INIT_H__
#define __ALOG_INIT_H__

/**
 * alog_early_init - will init the early for system eraly log.
 */
void alog_early_init(void);

/**
 * aipc_init - will int the alog module.
 */
int alog_init(void);

/**
 * aipc_init - will deint the alog module.
 */
int alog_deinit(void);

/**
 * alog_store_init - will deint the alog module.
 */
void alog_store_init(void);

/**
 * alog_store_deinit - will deint the alog module.
 */
void alog_store_deinit(void);

/**
 * alog_store_pre_init - will register the.fs mount event.
 */
void alog_store_pre_init(void);

#endif /* __ALOG_INIT_H__ */

