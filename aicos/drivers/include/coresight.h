/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines coresight driver api.
 */

#ifndef __CORESGITH_H__
#define __CORESGITH_H__

/* coresight trace mode */
typedef enum {
    TRACE_FIFO = 0,
    TRACE_LEASH,
    TRACE_STACK
} TRACE_MODE;

void aic_cs_trace_start(TRACE_MODE mode);
void aic_cs_trace_stop(void);
#ifdef CONFIG_DRIVER_CORESIGHT_MTB
void aic_cs_mtb_buf_early_resume(void);
#else
static inline void aic_cs_mtb_buf_early_resume(void){}
#endif
#endif /* __CORESGITH_H__ */

