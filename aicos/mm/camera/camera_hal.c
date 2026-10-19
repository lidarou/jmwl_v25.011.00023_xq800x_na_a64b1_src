/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b1ecc
#define LOG_MODULE    AMOD_CAMERA

#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <aos_thread.h>

#include <cache.h>
#include <core_op.h>

#include <csi.h>
#include <display.h>
#include <display_cfg.h>

#include <timer_inc.h>
#include <slp_inc.h>

#include <sensor_private.h>

#include "camera_hal.h"

#define CAMERA_THREAD_PRIO        15

static aos_thread_id camera_thread_id = AOS_INVALID_THREAD_ID;
static aos_mutex_ptr camera_mutex_ptr = AIC_NULL;

struct stream_info g_capture_info;
/* save preview info */
struct stream_info g_preview_info;

struct camera_info g_camera_info = {
    .cam_state = CAM_IN_CLOSE,
};
#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
#define CAMERA_VIDEO_QUEUE_NUM    1

struct stream_info g_video_info;
aos_queue_ptr camera_video_queue_ptr = AIC_NULL;
#endif

preview_refresh_cb g_preview_refresh = NULL;

#define CAMERA_PREVIEW_SIZE_IN_WORD        38400
u32 csi_rgb565_dst[CAMERA_PREVIEW_SIZE_IN_WORD] __attribute__((aligned(CACHE_LINE_SIZE))); ;

int camera_scaler(struct stream_info *cam_stream_ptr,
                  unsigned char* dst_ptr, struct camera_fb_info *fb_info_ptr,
                  enum YUYV_MODE yuyv_in_mode)
{
    int src_width, src_height, dst_width, dst_height, ret;
    camera_data_fmt_e fmt;
    yuv_plane_num_e plane_num;
    enum SCL_MODE scl_mode;

    src_width = cam_stream_ptr->src_width;
    src_height = cam_stream_ptr->src_height;
    dst_width = cam_stream_ptr->width;
    dst_height = cam_stream_ptr->height;
    fmt = cam_stream_ptr->fmt;
    plane_num = cam_stream_ptr->plane_num;

    /* set scaler mode */
    if ((src_width / dst_width == 2) && (src_height / dst_height == 2))
        scl_mode = BINNING;
    else if ((src_width <= dst_width) && (src_height <= dst_height))
        scl_mode = NO_SCALER;
    else
        scl_mode = BILINEAR_SCALER;

    aicx_info(0x1410bc, "d", "scl mode is %d\n", scl_mode);

    if (scl_mode == NO_SCALER) {
        aos_memcpy((void *)dst_ptr, (void *)fb_info_ptr->fb_addr, cam_stream_ptr->img_size);
        aicx_info(0x14722e, "", "no scaler, just copy to dst!\n");
        return AIC_SUCCESS;
    }

    switch (fmt) {
    case YUV422:
        if (plane_num == 2) {
            ret = yuv422_scaler((unsigned char *)fb_info_ptr->fb_addr,
                                (unsigned char *)fb_info_ptr->fb_addr + src_width * src_height,
                                src_width, src_height,
                                dst_ptr, dst_ptr + dst_width * dst_height,
                                dst_width, dst_height,
                                yuyv_in_mode, scl_mode, yuyv_in_mode);
            if (ret < 0) {
                aicx_error(0x0fa644, "", "yuv422 2 plane scaling failed!\n");
                return ret;
            }
            aicx_info(0x1501e1, "", "yuv422 2 plane scaler success!\n");
        } else {
            ret = yuv422_scaler((unsigned char *)fb_info_ptr->fb_addr, AIC_NULL,
                                src_width, src_height,
                                dst_ptr, AIC_NULL, dst_width, dst_height,
                                yuyv_in_mode, scl_mode, yuyv_in_mode);
            if (ret < 0) {
                aicx_error(0x1266b7, "", "yuv422 1 plane scaling failed!\n");
                return ret;
            }
            aicx_info(0x074ff8, "", "yuv422 1 plane scaler success!\n");
        }
        break;

    case YUV420:
        ret = yuv420_scaler((unsigned char *)fb_info_ptr->fb_addr,
                            (unsigned char *)fb_info_ptr->fb_addr + src_width * src_height,
                            src_width, src_height,
                            dst_ptr, dst_ptr + dst_width * dst_height,
                            dst_width, dst_height,
                            yuyv_in_mode, scl_mode);
        if (ret < 0) {
            aicx_error(0x103617, "", "yuv420 y_uv scaling failed!\n");
            return ret;
        }
        aicx_info(0x0ce260, "", "yuv420 scaler success!\n");
        break;

    case Y:
        ret = yuv420_scaler((unsigned char *)fb_info_ptr->fb_addr,
                            AIC_NULL,
                            src_width, src_height,
                            dst_ptr, AIC_NULL,
                            dst_width, dst_height,
                            yuyv_in_mode, scl_mode);
        break;

    default:
        aicx_error(0x014966, "d", "format %d scaling not support!\n", fmt);
        return AIC_ENOTSUP;
    }

    aicx_info(0x1d99ed, "", "camera size scaler success!\n");

    return AIC_SUCCESS;
}

static void camera_thread_entry(void *data)
{
    struct camera_fb_info *fb_info_ptr = AIC_NULL;
    struct stream_info *cam_stream_ptr = AIC_NULL;
    u32 actual_flags;
    int i, ret;
#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
    struct camera_video_queue_item cvq_item;
    void *buffer;
#endif

    AIC_INT_SAVE_AREA

    while(1) {
        /* wait for csi notify fb valid */
        aos_get_event(g_camera_info.camera_fb_event_ptr, CAMERA_FB_VALID_FLAG,
                      AOS_AND_CLEAR, &actual_flags, AOS_WAIT_FOREVER);
        cam_stream_ptr = &g_camera_info.cam_stream_info;
        /* for single buffer */
        if (cam_stream_ptr->fb_num == 1)
            csi_stop();

        AIC_INT_DISABLE
        for (i = 0; i < cam_stream_ptr->fb_num; i++) {
            fb_info_ptr = camera_get_fb_info(i);
            /* csi may not fill the buffer finish */
            if (fb_info_ptr->fb_empty)
               continue;

            if (cam_stream_ptr->fb_num != 1) {
                if (csi_get_frame_addr_config() != fb_info_ptr->fb_addr) {
                    /* the buffer is finish success, can be flush to lcdc */
                    fb_info_ptr->fb_flushing = TRUE;
                    break;
                }
            }
        }
        AIC_INT_RESTORE
        if (cam_stream_ptr->fb_num != 1) {
            /* check whether both framebuffers are all empty */
            if ((fb_info_ptr == AIC_NULL) || (fb_info_ptr->fb_flushing == FALSE))
                continue;
        } else {
            if (fb_info_ptr == AIC_NULL)
                continue;
        }

        /* get mutex */
        ret = aos_get_mutex(camera_mutex_ptr, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);

        /* invalidate dirty cache */
        AIC_ASSERT(IS_ALIGNED(fb_info_ptr->fb_addr, CACHE_LINE_SIZE));
        dcache_inv_by_range(fb_info_ptr->fb_addr, cam_stream_ptr->src_img_size);

        if (g_camera_info.cam_state == CAM_IN_PREVIEW) {
#if 0
            /* set scaler mode */
            if ((cam_stream_ptr->src_width <= cam_stream_ptr->width) &&
                (cam_stream_ptr->src_height <= cam_stream_ptr->height))
                scl_mode = NO_SCALER;
            else if ((cam_stream_ptr->src_width / cam_stream_ptr->width == 2) &&
                     (cam_stream_ptr->src_height / cam_stream_ptr->height == 2))
                scl_mode = BINNING;
            else
                scl_mode = BILINEAR_SCALER;
            /* convert to rgb565 */
            yuv422_to_rgb565((unsigned char*)fb_info_ptr->fb_addr, AIC_NULL,
                             cam_stream_ptr->src_width, cam_stream_ptr->src_height,
                             (unsigned short*)csi_rgb565_dst,
                             cam_stream_ptr->width, cam_stream_ptr->height,
                             MODE_YUYV, scl_mode,
                             cam_stream_ptr->rot_mode);

            display_refresh_sync((u32)csi_rgb565_dst);
#endif
            if (NULL != g_preview_refresh) {
                g_preview_refresh(fb_info_ptr->fb_addr, cam_stream_ptr->src_width,
                                  cam_stream_ptr->src_height, cam_stream_ptr->fmt);
            } else {
                aicx_error(0x051f7c, "", "g_preview_refresh is null!\n");
            }
        }

        if (g_camera_info.cam_state == CAM_IN_CAPTURE) {
            if (cam_stream_ptr->fmt == YUV422) {
                ret = camera_scaler(cam_stream_ptr,
                                    (unsigned char *)g_camera_info.user_cap_addr,
                                    fb_info_ptr, MODE_YUYV);
            } else {
                ret = camera_scaler(cam_stream_ptr,
                                    (unsigned char *)g_camera_info.user_cap_addr,
                                    fb_info_ptr, MODE_Y);
            }
            if (ret < 0)
                goto BUFFER_FLUSH_DONE;
        }
#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
        if (g_camera_info.cam_state == CAM_IN_VIDEO) {
            buffer = aos_dma_malloc(cam_stream_ptr->img_size);
            if (buffer == AIC_NULL) {
                aicx_error(0x0063ba, "", "malloc camera video buffer failed!\n");
                goto BUFFER_FLUSH_DONE;
            }

            ret = camera_scaler(cam_stream_ptr, buffer, fb_info_ptr, MODE_YUYV);
            if (ret < 0) {
                aos_dma_free(buffer);
                buffer = AIC_NULL;
                goto BUFFER_FLUSH_DONE;
            }

            cvq_item.raw_img_addr = (u32)buffer;
            cvq_item.raw_img_size = cam_stream_ptr->img_size;
            cvq_item.raw_img_width = cam_stream_ptr->width;
            cvq_item.raw_img_height = cam_stream_ptr->height;
            cvq_item.data_fmt = cam_stream_ptr->fmt;
            ret = aos_send_to_queue(camera_video_queue_ptr, &cvq_item, AOS_NO_WAIT);
            if (ret) {
                aicx_warn(0x09ff20, "", "camera video queue is full!\n");
                aos_dma_free(buffer);
                buffer = AIC_NULL;
            }
        }
#endif

BUFFER_FLUSH_DONE:
        /* lcdc flush done */
        fb_info_ptr->fb_empty = TRUE;
        fb_info_ptr->fb_flushing = FALSE;

        if (g_camera_info.cam_state == CAM_IN_CAPTURE)
            aos_set_event(g_camera_info.camera_fb_event_ptr, CAMERA_CAPTURE_FLAG, AOS_OR);

        /* for single buffer */
        if (cam_stream_ptr->fb_num == 1)
            csi_start();

        aos_put_mutex(camera_mutex_ptr);
    }
}

int camera_open(camera_id_e id)
{
    int ret;
    struct camera_spi_sensor_ops *cs_ops_ptr = NULL;

    if (camera_mutex_ptr == AIC_NULL) {
        aicx_error(0x0d581d, "", "camera mutex is not initialized, cannot open camera!\n");
        return AIC_ENODEV;
    }
    ret = aos_get_mutex(camera_mutex_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(ret == AOS_SUCCESS);

    if (camera_is_opened()) {
        aicx_error(0x1d30c5, "", "camera had already been opened!\n");
        aos_put_mutex(camera_mutex_ptr);

        return AIC_EIO;
    }

    cs_ops_ptr = camera_get_spi_sensor_ops(id);
    if (cs_ops_ptr == NULL) {
    /* camera_open may be called before camera_init */
        aicx_info(0x16d8e7, "d", "camera %d is not ready!\n", id);
        aos_put_mutex(camera_mutex_ptr);
        return AIC_EIO;
    }

    ret = cs_ops_ptr->cs_open();
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0526a5, "d", "camera %d power on failed!\n", id);
        aos_put_mutex(camera_mutex_ptr);
        return ret;
    }

    g_camera_info.camera_fb_event_ptr = aos_create_event("camera fb event");

    camera_thread_id = aos_create_thread_simply("camera thread",
                                                 camera_thread_entry,
                                                 NULL, SIZE_2K, CAMERA_THREAD_PRIO);

    aic_get_wakelock();
    g_camera_info.cam_state = CAM_IN_OPEN;

    aos_put_mutex(camera_mutex_ptr);

    aicx_info(0x0da614, "d", "camera %d power on success!\n", id);

    return ret;
}

void camera_close(camera_id_e id)
{
    int ret;
#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
    struct camera_video_queue_item cv_queue_item = {0};
#endif

    struct camera_spi_sensor_ops *cs_ops_ptr;

    if (aos_is_thread_created(camera_thread_id))
        if (aos_is_thread_suspended(camera_thread_id))
            aos_resume_thread(camera_thread_id);

    if (camera_mutex_ptr == AIC_NULL) {
        aicx_error(0x0074af, "", "camera mutex is not initialized, cannot close camera!\n");
        return;
    }
    ret = aos_get_mutex(camera_mutex_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(ret == AOS_SUCCESS);
    if (!camera_is_opened()) {
        aicx_error(0x16f50d, "", "camera had already been closed!\n");
        aos_put_mutex(camera_mutex_ptr);
        return;
    }

    cs_ops_ptr = camera_get_spi_sensor_ops(id);
    cs_ops_ptr->cs_close();

    if (aos_is_thread_created(camera_thread_id)) {
        aos_terminate_thread(camera_thread_id);
        aos_delete_thread(camera_thread_id);
    }

    g_camera_info.cam_state = CAM_IN_CLOSE;

    aos_delete_event(g_camera_info.camera_fb_event_ptr);

    aos_memset(&g_camera_info, 0, sizeof(g_camera_info));
    aos_memset(&g_capture_info, 0, sizeof(g_capture_info));
    aos_memset(&g_preview_info, 0, sizeof(g_preview_info));
#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
    aos_memset(&g_video_info, 0, sizeof(g_video_info));

    if (camera_video_queue_ptr != AIC_NULL) {
        ret = aos_recv_from_queue(camera_video_queue_ptr, &cv_queue_item, AOS_NO_WAIT);
        if (ret == AIC_SUCCESS) {
            aos_dma_free(cv_queue_item.raw_img_addr);
            cv_queue_item.raw_img_addr = 0;
            cv_queue_item.raw_img_size = 0;
        }
        aos_delete_queue(camera_video_queue_ptr);
        camera_video_queue_ptr = AIC_NULL;
    }
#endif


    csi_disable();
    aos_put_mutex(camera_mutex_ptr);

    aic_put_wakelock();
    aicx_info(0x08a688, "d", "camera %d power off success!\n", id);
}

int camera_preview(camera_id_e id, yuv_plane_num_e plane_num,
                   camera_data_fmt_e data_fmt,
                   int width, int height)
{
    int ret;
    struct camera_fb_info *cfb_info_ptr;
    struct camera_spi_sensor_ops *ops_ptr;

    if (!camera_is_opened()) {
        aicx_info(0x0e3fa8, "", "camera is not opened, cannot enter preview!\n");
        return AIC_EIO;
    }

    if (aos_is_thread_created(camera_thread_id))
        if (aos_is_thread_suspended(camera_thread_id))
            aos_resume_thread(camera_thread_id);

    ret = aos_get_mutex(camera_mutex_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(ret == AOS_SUCCESS);

    camera_ioctl(id, CAM_STREAM_OFF, AIC_NULL, AIC_NULL);

    if ((data_fmt != YUV422) && (plane_num == YUV_TWO_PLANE)) {
        aicx_error(0x1a72cb, "", " only YUV data format support two plane mode!\n");
        aos_put_mutex(camera_mutex_ptr);
        return AIC_EINVAL;
    }

    g_preview_info.fmt = data_fmt;
    g_preview_info.plane_num = plane_num;

    /* one pixel contains two bytes */
    if ((data_fmt == YUV422) ||  (data_fmt == RGB565))
        g_preview_info.bytes_per_pixel = 2;
    else
        g_preview_info.bytes_per_pixel = 1;

    g_preview_info.width = width;
    g_preview_info.height = height;
    g_preview_info.img_size = width * height * g_preview_info.bytes_per_pixel;

    g_preview_info.fb_num = camera_fb_alloc(g_preview_info.src_img_size);
    csi_set_addr_limit(g_preview_info.src_img_size);

    csi_output_plane_config(plane_num);

    cfb_info_ptr = camera_get_fb_info(0);
    csi_frame_addr_config(cfb_info_ptr->fb_addr);
    if (plane_num == YUV_TWO_PLANE) {
        csi_frame_addr2_config(cfb_info_ptr->fb_addr + cfb_info_ptr->fb_size/2);
        csi_endian_mode_config(1);
    }

    aos_memcpy(&g_camera_info.cam_stream_info, &g_preview_info, sizeof(g_preview_info));

    ops_ptr = camera_get_spi_sensor_ops(id);
    ret = ops_ptr->cs_set_output_format(g_preview_info.fmt);
    if (ret < 0) {
        aos_put_mutex(camera_mutex_ptr);
        return ret;
    }

    /*ret = ops_ptr->cs_set_output_size(g_preview_info.width, g_preview_info.height);
    if (ret < 0)
        return ret;*/
    //ret = ops_ptr->initial_setting(g_preview_info.fmt, g_preview_info.width, g_preview_info.height);

    g_camera_info.cam_state = CAM_IN_PREVIEW;

    camera_ioctl(id, CAM_STREAM_ON, AIC_NULL, AIC_NULL);
    aos_put_mutex(camera_mutex_ptr);

    aicx_info(0x0b287e, "d", "camera %d preview setting success!\n", id);

    return ret;
}

int camera_capture_prepare(camera_id_e id, yuv_plane_num_e plane_num,
                           camera_data_fmt_e data_fmt,
                           int width, int height)
{
    if ((data_fmt != YUV422) && (plane_num == YUV_TWO_PLANE)) {
        aicx_error(0x1a72cb, "", " only YUV data format support two plane mode!\n");
        return AIC_EINVAL;
    }

    aos_memcpy(&g_capture_info, &g_preview_info, sizeof(g_capture_info));

    g_capture_info.fmt = data_fmt;
    g_capture_info.plane_num = plane_num;
    //g_camera_info.cam_state = CAM_IN_CAPTURE;

    /* one pixel contains two bytes */
    if ((data_fmt == YUV422) ||  (data_fmt == RGB565))
        g_capture_info.bytes_per_pixel = 2;
    else
        g_capture_info.bytes_per_pixel = 1;

    g_capture_info.width = width;
    g_capture_info.height = height;
    g_capture_info.img_size = width * height * g_capture_info.bytes_per_pixel;

    aicx_info(0x186d8b, "d", "camera %d capture setting success!\n", id);

    return AIC_SUCCESS;
}

/* should stream off first */
int camera_capture(camera_id_e id, u32 user_addr)
{
    int ret;
    u32 actual_flags;
    struct camera_fb_info *cfb_info_ptr;
    struct camera_spi_sensor_ops *ops_ptr;

    if (!camera_is_opened()) {
        aicx_info(0x0a1802, "", "camera is not opened, cannot enter capture!\n");
        return AIC_EIO;
    }

    if (aos_is_thread_created(camera_thread_id))
        if (aos_is_thread_suspended(camera_thread_id))
            aos_resume_thread(camera_thread_id);

    ret = aos_get_mutex(camera_mutex_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(ret == AOS_SUCCESS);

    /* enable capture mode */
    camera_ioctl(id, CAM_STREAM_OFF, AIC_NULL, AIC_NULL);

    csi_cap_mode_enable(1);

    g_camera_info.user_cap_addr = user_addr;

    g_capture_info.fb_num = camera_fb_alloc(g_capture_info.src_img_size);

    csi_output_plane_config(g_capture_info.plane_num);

    cfb_info_ptr = camera_get_fb_info(0);
    csi_frame_addr_config(cfb_info_ptr->fb_addr);
    if (g_capture_info.plane_num == YUV_TWO_PLANE) {
        csi_frame_addr2_config(cfb_info_ptr->fb_addr + cfb_info_ptr->fb_size/2);
        csi_endian_mode_config(1);
    }

    aos_memcpy(&g_camera_info.cam_stream_info, &g_capture_info, sizeof(g_capture_info));
    ops_ptr = camera_get_spi_sensor_ops(id);
    ret = ops_ptr->cs_set_output_format(g_capture_info.fmt);
    if (ret < 0) {
        aos_put_mutex(camera_mutex_ptr);
        return ret;
    }

    csi_set_addr_limit(g_capture_info.src_img_size);

    ret = ops_ptr->cs_set_output_size(g_capture_info.width, g_capture_info.height);
    if (ret < 0) {
        aos_put_mutex(camera_mutex_ptr);
        return ret;
    }

    g_camera_info.cam_state = CAM_IN_CAPTURE;

    camera_ioctl(id, CAM_STREAM_ON, AIC_NULL, AIC_NULL);

    aos_put_mutex(camera_mutex_ptr);

    ret = aos_get_event(g_camera_info.camera_fb_event_ptr, CAMERA_CAPTURE_FLAG,
                  AOS_AND_CLEAR, &actual_flags, AOS_WAIT_FOREVER);
    if (ret < 0) {
        aicx_error(0x1040b2, "", "camera capture get CAMERA_CAPTURE_FLAG failed!\n");
        return ret;
    }

    aicx_info(0x15ea97, "", "camera capture done!\n");

    ret = aos_get_mutex(camera_mutex_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(ret == AOS_SUCCESS);

    camera_ioctl(id, CAM_STREAM_OFF, AIC_NULL, AIC_NULL);

    csi_cap_mode_enable(0);

    aos_put_mutex(camera_mutex_ptr);

    return ret;
}

#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
int camera_video_mode(camera_id_e id, int width, int height, camera_data_fmt_e fmt,
                      yuv_plane_num_e plane_num, u8 bytes_per_pixel)
{
    int ret;
    struct camera_fb_info *cfb_info_ptr;
    struct camera_spi_sensor_ops *ops_ptr;

    if (!camera_is_opened()) {
        aicx_info(0x061518, "", "camera is not opened, cannot enter video mode!\n");
        return AIC_EIO;
    }

    if (aos_is_thread_created(camera_thread_id))
        if (aos_is_thread_suspended(camera_thread_id))
            aos_resume_thread(camera_thread_id);

    ret = aos_get_mutex(camera_mutex_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(ret == AOS_SUCCESS);

    /* init g_video_info */
    aos_memcpy(&g_video_info, &g_preview_info, sizeof(g_video_info));
    g_video_info.fmt = fmt;
    g_video_info.plane_num = plane_num;
    g_video_info.bytes_per_pixel = bytes_per_pixel;

    g_video_info.width = width;
    g_video_info.height = height;
    g_video_info.img_size = width * height * g_video_info.bytes_per_pixel;

    /* enable video mode */
    camera_ioctl(id, CAM_STREAM_OFF, AIC_NULL, AIC_NULL);

    g_video_info.fb_num = camera_fb_alloc(g_video_info.src_img_size);

    cfb_info_ptr = camera_get_fb_info(0);
    csi_output_plane_config(plane_num);
    csi_frame_addr_config(cfb_info_ptr->fb_addr);
    if (plane_num == YUV_TWO_PLANE) {
        csi_frame_addr2_config(cfb_info_ptr->fb_addr + cfb_info_ptr->fb_size/2);
        csi_endian_mode_config(1);
    }

    aos_memcpy(&g_camera_info.cam_stream_info, &g_video_info, sizeof(g_video_info));
    ops_ptr = camera_get_spi_sensor_ops(id);
    ret = ops_ptr->cs_set_output_format(g_video_info.fmt);
    if (ret < 0)
        return ret;

    csi_set_addr_limit(g_video_info.src_img_size);

    ret = ops_ptr->cs_set_output_size(g_video_info.width, g_video_info.height);
    if (ret < 0)
        return ret;

    g_camera_info.cam_state = CAM_IN_VIDEO;

    if (!camera_video_queue_ptr)
        /* create a queue to save raw image */
        camera_video_queue_ptr = aos_create_queue("camera-video-queue",
                                                  sizeof(struct camera_video_queue_item),
                                                  CAMERA_VIDEO_QUEUE_NUM);

    camera_ioctl(id, CAM_STREAM_ON, AIC_NULL, AIC_NULL);
    aos_put_mutex(camera_mutex_ptr);

    return AIC_SUCCESS;
}

int camera_video(camera_id_e id, int width, int height)
{
    int ret;

    ret = camera_video_mode(id, width, height, YUV422, YUV_ONE_PLANE, 2);

    return ret;
}
#endif

int camera_set_fps(camera_id_e id, u8 fps)
{
    return AIC_SUCCESS;
}

u8 camera_get_fps(camera_id_e id)
{
    return 30;
}

int camera_get_sensor_output_format(camera_id_e id, camera_data_fmt_e *fmt)
{
    struct camera_spi_sensor_ops *cs_ops_ptr = AIC_NULL;

    cs_ops_ptr = camera_get_spi_sensor_ops(id);
    if (cs_ops_ptr == AIC_NULL) {
        aicx_error(0x050165, "d", "camera %d cannot get sensor ops!\n", id);
        return AIC_ENODEV;
    }

    if (cs_ops_ptr->cs_get_output_format == AIC_NULL) {
        aicx_error(0x1b2d0a, "d", "camera %d not support get output format!\n", id);
        return AIC_EIO;
    }

    *fmt = cs_ops_ptr->cs_get_output_format();

    return AIC_SUCCESS;
}

int camera_ioctl(camera_id_e id, camera_cmd_e cmd, void *input, void *output)
{
    int ret = AIC_SUCCESS;

    switch (cmd) {
    case CAM_STREAM_ON:
        ret = csi_start();
        break;

    case CAM_STREAM_OFF:
        csi_stop();
        break;

    /*case CAM_CAPTURE:
        ret = camera_capture(id);
        break;*/

    default:
        aicx_info(0x0dc8cf, "", "does not support the cmd!\n");
        ret = AIC_EINVAL;
        break;
    }

    return ret;
}

struct stream_info *camera_get_stream_info(stream_info_id_e id)
{
    switch (id) {
    case CAM_STREAM_CAPTURE_INFO:
        return &g_capture_info;

#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
    case CAM_STREAM_VIDEO_INFO:
        return &g_video_info;
#endif

    default:
        return &g_preview_info;
    }
}

int camera_register(preview_refresh_cb refresh)
{
    g_preview_refresh = refresh;

    return AIC_SUCCESS;
}

int camera_init(void)
{
    /* create camera lock */
    camera_mutex_ptr = aos_create_mutex("camera_mutex");
    AIC_ASSERT(camera_mutex_ptr);

    return AIC_SUCCESS;
}

AIC_NORMAL_DRV_L2_INIT_EXPORT(camera_init);

#if defined(CONFIG_CAMERA_OPEN_BY_DEFAULT)
int camera_open_by_default(void)
{
    int ret;

    ret = camera_open(CAMERA_FRONT);
    if (ret == AIC_SUCCESS)
        camera_preview(CAMERA_FRONT, YUV_ONE_PLANE, YUV422, LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT);
    else
        return ret;

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_L2_INIT_EXPORT(camera_open_by_default);
#endif

