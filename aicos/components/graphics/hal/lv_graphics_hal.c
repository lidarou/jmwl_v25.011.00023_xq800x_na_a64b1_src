/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *  This file is the gx316 SoC display interface register to lvgl subsystem.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1255bd
#define LOG_MODULE    AMOD_LVGL
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_at_resp_handle.h>
#include "lv_graphics_hal.h"
#include "lvgl.h"
#include <display.h>

/*TODO:later to delete this var*/
static boolean g_is_need_refresh = true;
static boolean lvgl_flush = true;

static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

/*TODO:later to delete this API*/
void lv_set_refresh(boolean enable)
{
    g_is_need_refresh = enable;
}

void lv_disp_init(void)
{
    /*Initialize device display*/
    disp_init();

    /*Create a buffer for drawing*/
    static lv_disp_draw_buf_t draw_buf_dsc;
    static lv_color_t buf_3_1[DISP_HOR_RES * DISP_VER_RES] __attribute__((aligned(32U)));            /*A screen sized buffer*/

#ifdef CONFIG_SYNC_FLIP
    lv_disp_draw_buf_init(&draw_buf_dsc, buf_3_1, NULL, DISP_HOR_RES * DISP_VER_RES);/*Initialize the display buffer*/
#else
    static lv_color_t buf_3_2[DISP_HOR_RES * DISP_VER_RES] __attribute__((aligned(32U)));            /*An other screen sized buffer*/
    lv_disp_draw_buf_init(&draw_buf_dsc, buf_3_1, buf_3_2, DISP_HOR_RES * DISP_VER_RES);/*Initialize the display buffer*/
#endif

    /*
    * Register the display in LVGL
    */

    /*Descriptor of a display driver*/
    static lv_disp_drv_t disp_drv;
    /*Basic initialization*/
    lv_disp_drv_init(&disp_drv);

    /*Set the resolution of the display*/
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc;

    disp_drv.full_refresh = 1;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/*Initialize gx316 display and the required peripherals.*/
static void disp_init(void)
{}

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if ((area->x2 < 0) || (area->y2 < 0) ||
        (area->x1 > (int32_t)DISP_HOR_RES - 1) || (area->y1 > (int32_t)DISP_VER_RES - 1)
        || !lvgl_flush) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    /*TODO:later to delete this judge*/
    if(g_is_need_refresh == true)
    {
        display_refresh((u32)color_p);
    }

    lv_disp_flush_ready(disp_drv);
}

static u32 at_lvgl_flush_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^LVGL_FLUSH");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "ON/OFF.");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_lvgl_flush_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    ulong rsp[1];
    int ret = 0;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LVGL_FLUSH=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (!strcmp((char *)paras_arr[0],"ON") || !strcmp((char *)paras_arr[0],"on")
        || !strcmp((char *)paras_arr[0],"On"))
        lvgl_flush = true;
    else if (!strcmp((char *)paras_arr[0],"OFF") || !strcmp((char *)paras_arr[0],"off")
        || !strcmp((char *)paras_arr[0],"Off"))
        lvgl_flush = false;
    else
        ret = AIC_ENXIO;

    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

AT_CMD_HANDLE lvgl_flush_at_handles[] = {
    {
        "AT^LVGL_FLUSH",
        at_lvgl_flush_test_cmd,
        AIC_NULL,
        at_lvgl_flush_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};


int lvgl_flush_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&lvgl_flush_at_handles[0]);

    return 0;
}
AIC_COMP_L2_INIT_EXPORT(lvgl_flush_at_cmd_init);
