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
#ifndef __ALOG_DATA_FORMATER_H__
#define __ALOG_DATA_FORMATER_H__

#include <aic_type.h>

void alog_data_format_ex(u8 sys, u8 data_type, u8 link, u8 *buf, size_t size, bool crc);
#define alog_data_format(t, l, b, s, c) alog_data_format_ex(AIC_THIS_SYS, t, l, b, s, c)

size_t alog_data_get_header_size(u8 data_type);
u8 alog_data_type_2_prot(u8 type);

#ifdef CONFIG_SUPPORT_ADSP
void alog_adsp_log_format(u8 *buf, size_t size, u32 sn, u8 b_aidx);
size_t alog_adsp_get_header_size(void);
#endif

#endif /* __ALOG_DATA_FORMATER_H__ */

