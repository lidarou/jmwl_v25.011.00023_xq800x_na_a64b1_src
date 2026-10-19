/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the flash management device drivers.
 */

#ifndef __FMDPART_H__
#define __FMDPART_H__

#include <aic_type.h>

char* fmd_get_dev_name(u8 devno);
int fmd_get_num_of_parts(u8 devno);
int fmd_get_num_of_groups(u8 devno);
int fmd_parse_partitions(struct fmd_dev *dev, int partno,
                         struct fmd_part *part);
struct fmd_pdev *fmd_allocate_partition(struct fmd_dev *dev,
                                        struct fmd_part *part,
                                        u32 cur_offset);
struct fmd_pdev *fmd_allocate_group(struct fmd_dev *dev, u32 gno);

#endif /* __FMDPART_H__ */

