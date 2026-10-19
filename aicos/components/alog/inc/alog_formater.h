/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log formater header file.
 *    ALFM_T: alog format.
 */
#ifndef __ALOG_FORMATER_H__
#define __ALOG_FORMATER_H__

#include <alog_inner.h>

enum {
    ALFMT_TRACE_FORMAT = 0,
    ALFMT_MSG_FORMAT,
};

typedef struct alog_formater {
    void (*format_data)(u32 sn, u8 *buf, size_t size);
    size_t (*get_header_size)(void);
} ALFM_T;

ALFM_T *alfm_create(u8 type);
void alfm_destroy(ALFM_T *alfm);

#endif /* __ALOG_FORMATER_H__ */

