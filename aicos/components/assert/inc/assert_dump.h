/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert dump memory related infomations.
 */

#ifndef __ASSERT_DUMP_H__
#define __ASSERT_DUMP_H__

#include <aic_type.h>
#include <assert_dump_api.h>
#include <diag_cmd_inner.h>
#include <aic_list.h>

#define ASSERT_DUMP_MAGIC_NUM 0x44554D50

struct assert_dump_head {
    u32 magic;
    enum assert_dump_type type;
    u32 address;
    u32 length;
};

struct assert_dump_info {
    enum assert_dump_type type;
#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
    u8  sys;
    u8  need_conf_mpu;
#endif
    u32 address;
    u32 length;
    dump_read_f read;
    dump_prepare_f pre_f;
    dump_post_f pst_f;
    dump_check_f check_f;
    u32 default_val;
    struct list_head node;
};

void assert_dump_handle(diag_recv_cmd_t *diag);
void assert_dump_region_init(void);

#endif /* __ASSERT_DUMP_H__ */
