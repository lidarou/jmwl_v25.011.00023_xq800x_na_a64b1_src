/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log store api header file.
 */
#ifndef __ALOG_STORE_API_H__
#define __ALOG_STORE_API_H__

void alog_store_save_log(void *data, u32 size);
void alog_store_save_dump(void *data, u32 size, bool finish);
void alog_store_save_assert_info(char *assert_info, size_t size, bool end_z);
bool alog_store_is_ready(void);

#endif /* __ALOG_STORE_API_H__ */
