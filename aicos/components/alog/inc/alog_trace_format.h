/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The trace log formater header file.
 */
#ifndef __ALOG_TRACE_FORMATER_H__
#define __ALOG_TRACE_FORMATER_H__
#include <aic_type.h>

void alog_trace_format(u32 sn, u8 *buf, size_t size);
size_t alog_trace_get_header_size(void);

#endif /* __ALOG_TRACE_FORMATER_H__ */

