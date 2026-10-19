/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the ftl random access drivers.
 */

#ifndef __FTLRANDOM_H__
#define __FTLRANDOM_H__

#include <aic_type.h>

int ftlrandom_open(char *name, u32 *handle);
int ftlrandom_close(u32 handle);
int ftlrandom_read(u32 handle, u32 offset, u32 size, u8 *buf);
int ftlrandom_write(u32 handle, u32 offset, u32 size, u8 *buf);
int ftlrandom_get_sector_size(u32 handle, u32 *retsize);
int ftlrandom_get_sector_num(u32 handle, u32 *retnum);
int ftlrandom_get_partition_size(u32 handle, u32 *retsize);

#endif /* __FMDRANDOM_H__ */

