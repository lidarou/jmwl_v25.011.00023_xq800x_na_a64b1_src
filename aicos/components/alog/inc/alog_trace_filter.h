/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The aic log filter header file.
 */
#ifndef __ALOG_TRACE_FILTER_H__
#define __ALOG_TRACE_FILTER_H__

#include <aic_type.h>

void alog_trace_filter_init(void);
void alog_trace_filter_init_fast(void);
u8 alog_get_log_level(u8 mod);
void alog_set_log_level(u8 mod, u8 level);
void alog_just_set_log_level(u8 mod, u8 level);
bool alog_trace_filter_out(u8 mod, u8 level);
void alog_set_fast_log(u8 enable);
#endif /* __ALOG_TRACE_FILTER_H__ */

