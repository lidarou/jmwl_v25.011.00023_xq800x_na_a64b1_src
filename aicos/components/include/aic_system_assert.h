/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert related infomations.
 */

#ifndef __AIC_SYSTEM_ASSERT_H__
#define __AIC_SYSTEM_ASSERT_H__

void assert_passive_handle(void);
void aic_sys_assert(char *fmt, ...);
void aic_sys_assert_ex(char *file, unsigned int line);
void aic_sys_assert_idx(unsigned int fileid, unsigned int line);
void aic_sys_assert_info(char *file, unsigned int line, char *fmt, ...);
void aic_sys_assert_info_idx(unsigned int fileid, unsigned int line,
                             unsigned int fmtid, const char *pf, ...);
void aic_sys_assert_info_idx_s(unsigned int fileid, unsigned int line, char *fmt, ...);
_Bool aic_sys_is_assert(void);
void aic_assert_set_queue_full(void);

#endif /* __AIC_SYSTEM_ASSERT_H__ */
