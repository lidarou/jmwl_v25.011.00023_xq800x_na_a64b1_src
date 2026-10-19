/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is display at cmd handle.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x106ac0
#define LOG_MODULE    AMOD_GPAPH
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aic_at_resp_handle.h>
#include <aic_bitmap.h>
#include <cache.h>
#include <display.h>
#include <display_cfg.h>
#include <fmd.h>
#include <irq_chip.h>
#include <pmu_user.h>
#include <timer_inc.h>

#define DISPLAY_MAX_AT_RESP_LENGTH        64UL
#define DISPLAY_REPLY_LINE_COUNT_ONCE     2UL

struct pmu_user *at_backlight_handle = NULL;

u32 aic_display_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[DISPLAY_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[DISPLAY_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, DISPLAY_MAX_AT_RESP_LENGTH, "+SHOWLOGO");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 aic_screenon_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[DISPLAY_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[DISPLAY_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, DISPLAY_MAX_AT_RESP_LENGTH, "+SCREENON");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 aic_screenoff_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[DISPLAY_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[DISPLAY_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, DISPLAY_MAX_AT_RESP_LENGTH, "+SCREENOFF");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 aic_display_showlogo_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[3] = {'\0'};
    ulong line_pointers[DISPLAY_REPLY_LINE_COUNT_ONCE];

#ifdef CONFIG_DISPLAY_SHOWLOGO
    extern int show_logo();
    show_logo();
#endif

    strcpy(result_line0, "OK");
    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

int aic_at_screen_on(void)
{
    s32 ret = AIC_SUCCESS;

    display_resume();
    ret = aic_bl_brightness_set(100);
    if (ret)
        aicx_info(0x1620b1, "x", "aic_at_screen_on:failed(0x%x)!\n", ret);
    return ret;
}

int aic_at_screen_off(void)
{
    s32 ret = AIC_SUCCESS;

    ret = aic_bl_brightness_set(0);
    if (ret)
        aicx_info(0x0ae38f, "x", "aic_at_screen_off:failed(0x%x)!\n", ret);
    display_suspend();

    return ret;
}

u32 aic_display_screenon_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong line_pointers[DISPLAY_REPLY_LINE_COUNT_ONCE];
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    int ret;

    ret = aic_at_screen_on();
    if (ret) {
        aicx_warn(0x1534f3, "d", "at turn on screen fail! ret = %d \n",ret);
        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", ret);
    } else {
        strcpy(result_line0, "OK");
    }

    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

u32 aic_display_screenoff_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong line_pointers[DISPLAY_REPLY_LINE_COUNT_ONCE];
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    int ret;

    ret = aic_at_screen_off();
    if (ret) {
        aicx_warn(0x104782, "d", "at turn off screen fail! ret = %d \n",ret);
        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", ret);
    } else {
        strcpy(result_line0, "OK");
    }

    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

#ifdef  CONFIG_BLT_SUPPORT
/* *********************************************************************
 * AT^LCDCTRL cmd start
 ***********************************************************************/
#define PURE_RED    0xf800
#define PURE_GREEN  0x400
#define PURE_BLUE   0x1f
#define PURE_WHITE  0xffff
#define PURE_BLACK  0x0

static u32 at_lcdctrl_test_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[128] = {0};
    int len;

    strcpy(rsp_str, "^LCDCTRL");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "R/G/B/W/D(Red/Green/Blue/White/Black)");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static void display_buffer_fill(uint16_t (*pframe)[LCD_PANEL_WIDTH], uint16_t rgb565_color)
{
    int i, j;

    for( i = 0; i < LCD_PANEL_HEIGHT; i++)
        for (j = 0; j < LCD_PANEL_WIDTH; j++)
            pframe[i][j] = rgb565_color;
}

static int lcd_ctrl_fill_color(u16 color)
{
    char *display_buf;

    display_buf = aos_zalloc(LCD_PANEL_HEIGHT * LCD_PANEL_WIDTH * 2);
    if (!display_buf) {
        aicx_warn(0x09ecae, "", "alloc display buffer fail!!\n", __FILE__, __LINE__, __func__);
        return AIC_ENXIO;
    }

    display_buffer_fill((uint16_t (*)[])display_buf, color);
    display_refresh((u32)display_buf);

    aos_free(display_buf);

    return AIC_SUCCESS;
}

static u32 at_lcdctrl_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    int ret = 0;
    u16 pure_color;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LCDCTRL=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (!strcmp((char *)paras_arr[0],"R") || !strcmp((char *)paras_arr[0],"r"))
        pure_color = PURE_RED;
    else if (!strcmp((char *)paras_arr[0],"G") || !strcmp((char *)paras_arr[0],"g"))
        pure_color = PURE_GREEN;
    else if (!strcmp((char *)paras_arr[0],"B") || !strcmp((char *)paras_arr[0],"b"))
        pure_color = PURE_BLUE;
    else if (!strcmp((char *)paras_arr[0],"W") || !strcmp((char *)paras_arr[0],"w"))
        pure_color = PURE_WHITE;
    else if (!strcmp((char *)paras_arr[0],"D") || !strcmp((char *)paras_arr[0],"d"))
        pure_color = PURE_BLACK;
    else
        ret = AIC_ENXIO;

    if (ret) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    aic_at_screen_on();

    ret = lcd_ctrl_fill_color(pure_color);
    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}


u32 aic_lcdoff_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[DISPLAY_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[DISPLAY_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, DISPLAY_MAX_AT_RESP_LENGTH, "^LCDOFF");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

static u32 at_lcdoff_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^LCDOFF");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "Turn off lcdc & backlight");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_lcdoff_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    return aic_display_screenoff_cmd(at_req_res_ptr);
}

static u32 at_bl_ctrl_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^BACKLIGHTCTRL");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "1/2/..99/100(percentage of maximum brightness).");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_bl_ctrl_query_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[30] = {0};
    ulong rsp[2];
    int len, ret;
    u32 value;

    ret = aic_bl_brightness_get(&value);
    rsp[0] = (ulong)rsp_str;

    strcpy(rsp_str, "^BACKLIGHTCTRL");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';

    if (ret) {
        sprintf(rsp_str + len, "err=%d", ret);
        rsp[1] = (ulong)AT_SIMPLE_RESULT_ERROR;
    } else {
        sprintf(rsp_str + len, "%d", value);
        rsp[1] = (ulong)AT_RESULT_OK;
    }

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_bl_ctrl_cmd(AT_REQ_RESP *at_req)
{
    u8 brightness = 0;
    ulong *paras_arr;
    ulong rsp[1];
    int ret = 0;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^BACKLIGHTCTRL=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    brightness = atoi((u8 *)paras_arr[0]);

    if (brightness > 100) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    display_resume();
    ret = lcd_ctrl_fill_color(PURE_WHITE);
    if (ret) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    ret = aic_bl_brightness_set(brightness);
    if (ret < 0)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

#endif

AT_CMD_HANDLE display_at_handles[] = {
    {"AT+SHOWLOGO", aic_display_test_cmd, AIC_NULL,
     aic_display_showlogo_cmd, AIC_NULL},
    {"AT+SCREENON", aic_screenon_test_cmd, AIC_NULL,
     aic_display_screenon_cmd, AIC_NULL},
    {"AT+SCREENOFF", aic_screenoff_test_cmd, AIC_NULL,
     aic_display_screenoff_cmd, AIC_NULL},
#ifdef CONFIG_BLT_SUPPORT
    {
        "AT^LCDCTRL",
        at_lcdctrl_test_cmd,
        AIC_NULL,
        at_lcdctrl_cmd,
        AIC_NULL
    },
    {
        "AT^LCDOFF",
        at_lcdoff_test_cmd,
        AIC_NULL,
        at_lcdoff_cmd,
        AIC_NULL
    },
    {
        "AT^BACKLIGHTCTRL",
        at_bl_ctrl_test_cmd,
        at_bl_ctrl_query_cmd,
        at_bl_ctrl_cmd,
        AIC_NULL
    },
#endif
    {AIC_NULL},
};

int display_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&display_at_handles[0]);
    return 0;
}
AIC_COMP_L2_INIT_EXPORT(display_at_cmd_init);
