/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log trace header file.
 */
#ifndef __ALOG_DATA_H__
#define __ALOG_DATA_H__
#include <aic_osa.h>
#include <aipc_api.h>
#include <alog_api.h>
#include <alog_formater.h>
#include <alog_frame.h>

/* alog trace data */
typedef struct ALOG_DATA {
    ABRR_HANDLE     brr;
    u32             lost_cnt;
} ALOG_DATA_T;

ALOG_DATA_T *alog_data_create(u8 sys);
void alog_data_destroy(ALOG_DATA_T *alog_data);

#endif /* __ALOG_DATA_H__ */

