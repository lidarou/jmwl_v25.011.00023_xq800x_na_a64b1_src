/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The aic log message filter header file.
 */
#ifndef __ALOG_MSG_FILTER_H__
#define __ALOG_MSG_FILTER_H__

#include <aic_type.h>

bool alog_msg_filter_out(void *msg);
int alog_msg_filter_init(void);

#endif /* __ALOG_MSG_FILTER_H__ */

