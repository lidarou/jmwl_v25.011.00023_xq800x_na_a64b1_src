/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the ftl block access drivers.
 */

#ifndef __FTLBLOCK_H__
#define __FTLBLOCK_H__

#include <aic_type.h>

int ftlblock_open(char *name, u32 *handle);
int ftlblock_close(u32 handle);
int ftlblock_read(u32 handle, u32 sector_id, u32 sector_num, u8 *buf);
int ftlblock_write(u32 handle, u32 sector_id, u32 sector_num, void *buf);
int ftlblock_release(u32 handle, u32 sector_id, u32 sector_num);
int ftlblock_get_sector_size(u32 handle, u32 *retsize);
int ftlblock_get_sector_num(u32 handle, u32 *retnum);
int ftlblock_get_partition_size(u32 handle, u32 *retsize);
int ftl_init(void);

#endif /* __FTLBLOCK_H__ */
