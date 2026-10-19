/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file comp_camera_preview.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "camera_hal.h"
#include "comp_camera_preview.h"

/*********************
 *      DEFINES
 *********************/
#define COMP_PREVIEW_TIMEOUT     (30)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/


/**********************
 *  STATIC VARIABLES
 **********************/
extern u32 csi_rgb565_dst[];

/**********************
 *   STATIC FUNCTIONS
 **********************/
u32 g_need_refresh = 0;
u32 g_frame_addr = 0;
lv_timer_t * g_preview_timer = NULL;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_img_dsc_t g_img_dsc= {
  .header.always_zero = 0,
  .header.w = 0,
  .header.h = 0,
  .data_size = 0,
  .header.cf = LV_IMG_CF_TRUE_COLOR,
  .data = NULL,
};

static void preview_refresh(u32 frame_addr, int width, int height, camera_data_fmt_e format)
{
    enum SCL_MODE scl_mode;

    /* set scaler mode */
    if ((width <= g_img_dsc.header.w) &&
        (height <= g_img_dsc.header.h))
        scl_mode = NO_SCALER;
    else if ((width / g_img_dsc.header.w == 2) &&
             (height / g_img_dsc.header.h == 2))
        scl_mode = BINNING;
    else
        scl_mode = BILINEAR_SCALER;
    /* convert to rgb565 */
    yuv422_to_rgb565((unsigned char*)frame_addr, AIC_NULL,
                     width, height,
                     (unsigned short*)csi_rgb565_dst,
                     g_img_dsc.header.w, g_img_dsc.header.h,
                     MODE_YUYV, scl_mode,
                     0);

    g_frame_addr = (u32)csi_rgb565_dst;

    g_need_refresh = 1;
}

void pic_timer(lv_timer_t * t)
{
    lv_obj_t * preview_img = t->user_data;

    /*  timer callback中，判断需要刷新，则刷新 img 控件 */
    if (g_need_refresh) {/* g_need_refresh 这个 flag 在注册给 camera hal 的 callback 中设置为1，表示有数据需要刷新 */

        g_img_dsc.data = (const uint8_t *)g_frame_addr;
        lv_img_set_src(preview_img,&g_img_dsc);
        g_need_refresh = 0;

    }
}

/**
* comp_camera_preview_create - 创建 Camera preview 控件实例，当前只支持同时创建一个
* @parent: [IN] preview 控件的父对象指针.
* @src_width: [IN] preview 显示的宽度.
* @src_hight: [IN] preview 显示的高度.
* @bytes_per_pixel: [IN] 每个像素占用的Bytes空间.
* Return: 返回创建好的 preview 对象指针.
*/
lv_obj_t *comp_camera_preview_create(lv_obj_t *parent, u32 src_width, u32 src_hight, u32 bytes_per_pixel)
{
    lv_obj_t *preview_img = NULL;

    /* TODO:Use Camera service API to get preview frame addr */
    camera_register(preview_refresh);

    /* 1. 创建使用 LVGL 的 img 控件周期性绘制图片，图片数据源为 camera hal 的数据buffer */
    preview_img = lv_img_create(parent);

    g_img_dsc.header.w = src_width;
    g_img_dsc.header.h = src_hight;
    g_img_dsc.data_size = src_width * src_hight * bytes_per_pixel;
    g_img_dsc.data = (const uint8_t *)g_frame_addr;
    lv_img_set_src(preview_img, &g_img_dsc);

    /* 2. 设置周期性timer 刷新 img控件 */
    g_preview_timer = lv_timer_create(pic_timer, COMP_PREVIEW_TIMEOUT, preview_img);/* timeout 时间 <50ms, camera 刷新率是 20fps */
    lv_timer_set_repeat_count(g_preview_timer, -1);

    return preview_img;
}

/**
* comp_camera_preview_destroy - 销毁 Camera preview 控件实例
* @obj: [IN] 要销毁的 preview 对象指针.
* Return: 无.
*/
void comp_camera_preview_destroy(lv_obj_t *obj)
{
    g_frame_addr = 0;
    g_need_refresh = 0;

    lv_timer_del(g_preview_timer);
    lv_obj_del(obj);
}

void comp_camera_preview_pause(void)
{
    lv_timer_pause(g_preview_timer);
}

void comp_camera_preview_resume(void)
{
    lv_timer_resume(g_preview_timer);
}

