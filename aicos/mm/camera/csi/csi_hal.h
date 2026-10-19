/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __CSI_HAL_H__
#define __CSI_HAL_H__

#include <aic_type.h>
#include <csi.h>
#include <irq_chip.h>

extern u32 csi_framebuffer1[153600];
extern u32 csi_framebuffer2[153600];

struct camera_flush_info {
    u32 flush_addr;
    u32 flush_size;
    bool flushing;
    aos_queue_ptr queue_ptr;
    aos_thread_id camera_flush_pid;
};

ISR_RET_E csi_irq_handler(void *param);

#endif /* __CSI_HAL__ */

