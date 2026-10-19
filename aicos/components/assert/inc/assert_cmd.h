/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert command related infomations.
 */

#ifndef __ASSERT_CMD_H__
#define __ASSERT_CMD_H__

#include <aic_type.h>
#include <diag_cmd_inner.h>
#include <aic_list.h>
#include <system_assert.h>

typedef void (*assert_cmd_func)(void *p1, void *p2);

struct assert_cmd_info
{
    u16 cmd_code;
    assert_cmd_func func;
    char *description;
    struct list_head node;
};

void assert_diag_send(diag_recv_cmd_t *diag, void *data, size_t size);
void assert_cmd_process(struct assert_common_info *info);
void assert_cmd_init(void);

#endif /* __ASSERT_CMD_H__ */
