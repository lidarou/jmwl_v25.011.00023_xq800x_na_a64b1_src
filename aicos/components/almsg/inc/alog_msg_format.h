/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The msg log formater header file.
 */
#ifndef __ALOG_MSG_FORMATER_H__
#define __ALOG_MSG_FORMATER_H__

#include <aic_type.h>

typedef struct ALOG_MSG_FORMATER_OPS {
    /* msg_copy will alloc and copy, need to free it use msg_free. */
    void *(*msg_copy)(const void *msg);
    void (*msg_free)(void *msg);
    size_t (*msg_get_size)(void *msg);
    u32 (*msg_format)(void *msg, u32 offset, u8 *buf, size_t size);
} ALMF_OPS_T;

#ifdef CONFIG_FSMMSG_FEATURE
#include <alog_fsmmsg_format.h>

#define alog_msg_frame_format           alog_fsmmsg_frame_format
#define alog_msg_frame_get_header_size alog_fsmmsg_frame_get_header_size
#define alog_msg_get_formater_ops       alog_fsmmsg_get_formater_ops
#else
void alog_msg_frame_format(u32 sn, u8 *buf, size_t size);
size_t alog_msg_frame_get_header_size(void);
const ALMF_OPS_T *alog_msg_get_formater_ops(void);
#endif /* CONFIG_FSMMSG_FEATURE */

#endif /* __ALOG_MSG_FORMATER_H__ */

