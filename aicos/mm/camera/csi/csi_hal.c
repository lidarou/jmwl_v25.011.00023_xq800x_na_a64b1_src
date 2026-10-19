/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0446b5
#define LOG_MODULE    AMOD_CAMERA

#include <aic_errno.h>
#include <aic_seg_alloc.h>
#include <aic_init.h>
#include <aic_interrupt.h>
#include <aic_io.h>
#include <irq.h>
#include <tx_api.h>

#include <camera_hal.h>

#include "csi.h"
#include "csi_hal.h"

ISR_RET_E csi_irq_handler(void *param)
{
    u32 i, val;
    u32 fb_addr;
    u32 fb_size;
    boolean fb_valid;
    struct camera_fb_info *fb_info;
    struct stream_info *cam_stream_ptr;

    val = AIC_READ32((void *)REG_CSI_CSI_INT_OUT);

    if (g_camera_info.cam_state == CAM_IN_CAPTURE)
        cam_stream_ptr = camera_get_stream_info(CAM_STREAM_CAPTURE_INFO);
    else
        cam_stream_ptr = camera_get_stream_info(CAM_STREAM_PREVIEW_INFO);

    if (val & BIT_CSI_END_OF_FRAME) {
        if (is_csi_enabled()) {
            aos_set_event(g_camera_info.camera_fb_event_ptr, CAMERA_FB_VALID_FLAG, AOS_OR);

            fb_addr = 0;
            fb_size = 0;
            fb_valid = FALSE;
            for (i = 0; i < cam_stream_ptr->fb_num; i++) {
                fb_info = camera_get_fb_info(i);
                /* whether can set fb full */
                if (fb_info->fb_addr == csi_get_frame_addr_config()) {
                    fb_info->fb_empty = FALSE;
                    continue;
                }

                /* if the framebuffer is flushing, ignore it */
                if (!fb_info->fb_flushing) {
                    if (fb_valid == FALSE) {
                        fb_addr = fb_info->fb_addr;
                        fb_size = fb_info->fb_size;
                        fb_valid = TRUE;
                    }
                }
            }

            if ((fb_addr != 0) && (fb_size != 0)) {
                csi_stop();
                csi_frame_addr_config(fb_addr);
                AIC_ASSERT(csi_get_frame_addr_config() == fb_addr);
                if (cam_stream_ptr->plane_num == YUV_TWO_PLANE)
                    csi_frame_addr2_config(fb_addr + fb_size/2);
                csi_start();
            }
        }
        csi_int_clear_eof();
    }

    if (val & BIT_CSI_START_OF_FRAME) {
        csi_int_clear_sof();
    }

    if (val & BIT_CSI_START_RX_IS_FULL) {
        csi_int_clear_rxfifo_full();
    }

    if (val & BIT_CSI_TIMEOUT) {
        csi_int_clear_timeout();
    }

    if (val & BIT_CSI_EOL) {
        csi_int_clear_eol();
    }

    if (val & BIT_CSI_SOL) {
        csi_int_clear_sol();
    }

    return NONE_WORKER;
}

