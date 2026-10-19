/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    AIPC adpter header file.
 */

#ifndef __AIPC_ADAPTER_H__
#define __AIPC_ADAPTER_H__
#include <aipc_api.h>

void aipc_drv_reset_recvbox(u8 sys);
int aipc_drv_send_msg(u8 dst_sys, AMSG_T *amsg);
int aipc_drv_raw_send_msg(u8 dst_sys, AMSG_T *amsg);
int aipc_drv_register_msg(u8 dst_sys, AMSG_HANDLER_T handler, void *p);
void aipc_drv_unregister_msg(u8 dst_sys);
void aipc_write_flush_blk(ABLK_T *blk);
void aipc_read_flush_blk(ABLK_T *blk);

#endif /* __AIPC_ADAPTER_H__ */

