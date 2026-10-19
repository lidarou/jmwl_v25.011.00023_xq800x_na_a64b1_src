/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log formater source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07a888
#define LOG_MODULE    AMOD_ALOG

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <alog_formater.h>
#include <alog_data_format.h>
#include <alog_trace_format.h>
#include <alog_inner.h>
#include <stdio.h>

#ifdef CONFIG_ALOG_MSG_SUPPORT
#include <alog_msg_format.h>
#endif

ALFM_T *alfm_create(u8 type)
{
    ALFM_T *alfm;

    alfm = aos_malloc(sizeof(*alfm));
    AIC_ASSERT(alfm);

    switch(type) {
        case ALFMT_TRACE_FORMAT:
            alfm->format_data = alog_trace_format;
            alfm->get_header_size = alog_trace_get_header_size;
            break;

#ifdef CONFIG_ALOG_MSG_SUPPORT
        case ALFMT_MSG_FORMAT:
            alfm->format_data = alog_msg_frame_format;
            alfm->get_header_size = alog_msg_frame_get_header_size;
            break;
#endif

        default:
            AIC_DUMMY_ASSERT(0);
    }

    return alfm;
}

void alfm_destroy(ALFM_T *alfm)
{
    aos_free(alfm);
}

