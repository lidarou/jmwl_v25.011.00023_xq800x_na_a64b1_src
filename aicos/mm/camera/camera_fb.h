/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __CAMERA_FB_H__
#define __CAMERA_FB_H__

#include <aic_size.h>
#include <cache.h>
#include <camera_sensor_cfg/camera_fb_cfg.h>

#define CAMERA_FB_VALID_FLAG     0x1
#define CAMERA_FB_FLUSH_FLAG     0x2
#define CAMERA_CAPTURE_FLAG      0x4

struct camera_fb_info {
    volatile u32 fb_addr;
    volatile u32 fb_size;
    /* have valid data? */
    volatile bool fb_empty;
    /* using to flush to lcdc? */
    volatile bool fb_flushing;
};

u8 camera_fb_alloc(u32 fb_size);
struct camera_fb_info *camera_get_fb_info(int fb_id);

#endif

