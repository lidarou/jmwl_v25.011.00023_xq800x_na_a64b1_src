/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The data log formater source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x01b2a5
#define LOG_MODULE    AMOD_ALOG

#include <aic_checksum.h>
#include <alog_data_format.h>
#include <alog_frame.h>
#include <alog_api.h>
#include <alog_inner.h>
#include <stdio.h>

#define ALOG_ADSP_HEAD_FMT "\n[adsp][sn %010d][%04x]:"
#define ALOG_ADSP_HEADER_SIZE  30

static u32 s_data_sn[ALOG_DATA_CNT];

u8 alog_data_type_2_prot(u8 type)
{
    u8 prot = LOG_PRO_DATA_NORMAL;

    switch (type) {
    case ALOG_DATA_CAP:
        prot = LOG_PRO_NET_DATA_CAP;
        break;

    case ALOG_DATA_AUD:
        prot = LOG_PRO_AUD_DATA_PCM;
        break;
/*
    case ALOG_DATA_IQ:
        prot = LOG_PRO_IQ_DATA_LTE;
        break;
*/
    case ALOG_DATA_DUMP:
        prot = LOG_PRO_DATA_DUMP;
        break;

    case ALOG_DATA_NORMAL:
        prot = LOG_PRO_DATA_NORMAL;
        break;

    default:
        break;
    }

    return prot;
}

void alog_data_format_ex(u8 sys, u8 data_type, u8 link, u8 *buf, size_t size, bool crc)
{
    ALOG_FRAME_H_T *frame = (ALOG_FRAME_H_T *)buf;
    u8 prot = alog_data_type_2_prot(data_type);
    u16 sum = 0;

#ifdef CONFIG_SUPPORT_CHECK_SUM
    if (crc)
        sum = aic_calc_checksum((const void *)(frame + 1), size - ALOG_FRAME_H_SIZE);
#endif

    DATA_SET_FRAME_HEAD_EX(frame, sys, ++s_data_sn[data_type], size, prot, sum, link, SUB_PRO_SAVE);
}

size_t alog_data_get_header_size(u8 data_type)
{
    return ALOG_FRAME_H_SIZE;
}

#ifdef CONFIG_SUPPORT_ADSP
void alog_adsp_log_format(u8 *buf, size_t size, u32 sn, u8 b_aidx)
{
#ifdef CONFIG_ALOG_FRAME_SUPPORT
    ALOG_FRAME_H_T *frame = (ALOG_FRAME_H_T *)buf;
    u8 prot = b_aidx ? LOG_PRO_TRACE_ID : LOG_PRO_TRACE_TEXT;

    DATA_SET_FRAME_HEAD(frame, sn, size, prot, 0, 0, 0);

    /* change to adsp sys. */
    frame->sys = AIC_SYS_ADSP;

#else /* CONFIG_ALOG_FRAME_SUPPORT */
    memset(buf, 0, ALOG_ADSP_HEADER_SIZE);
    snprintf(buf, ALOG_ADSP_HEADER_SIZE, ALOG_ADSP_HEAD_FMT, sn, size);
    buf[ALOG_ADSP_HEADER_SIZE - 1] = '\n';
#endif /* CONFIG_ALOG_FRAME_SUPPORT */
}

size_t alog_adsp_get_header_size(void)
{
#ifdef CONFIG_ALOG_FRAME_SUPPORT
    return ALOG_FRAME_H_SIZE;
#else
    return ALOG_ADSP_HEADER_SIZE;
 #endif
}
#endif
