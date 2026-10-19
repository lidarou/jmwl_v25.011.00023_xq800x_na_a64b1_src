/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is enter of engineering mode calibration
*/



/*********************
 *      INCLUDES
*********************/

#include <aic_osa_ex.h>
#include "lvgl.h"
#include "alv_app_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#if CONFIG_TSC
#include "lv_port_indev_touchpad_cali.h"
#endif
/*********************
 *      DEFINES
*********************/
#define CALIBRAION_POINT_COUNT      (4)

#define CALI_POINT1_CENTER_X        (20)
#define CALI_POINT1_CENTER_Y        (20)

#define CALI_POINT2_CENTER_X        (220)
#define CALI_POINT2_CENTER_Y        (20)

#define CALI_POINT3_CENTER_X        (20)
#define CALI_POINT3_CENTER_Y        (300)

#define CALI_POINT4_CENTER_X        (220)
#define CALI_POINT4_CENTER_Y        (300)

#define CALI_BTN_SIZE               (39) /*must be odd number,because center has 1 pixel*/
#define CALI_LINE_HOR_SIZE          (19) /*must be odd number,because center has 1 pixel*/
#define CALI_LINE_VER_SIZE          (25) /*must be odd number,because center has 1 pixel*/


/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t *s_p_cont = NULL;
static lv_obj_t *s_p_line1_hor = NULL;
static lv_obj_t *s_p_line1_ver = NULL;
static lv_obj_t *s_p_btn1 = NULL;/*btn1 center(20,20)*/

static lv_obj_t *s_p_line2_hor = NULL;
static lv_obj_t *s_p_line2_ver = NULL;
static lv_obj_t *s_p_btn2 = NULL;/*btn2 center(220,20)*/

static lv_obj_t *s_p_line3_hor = NULL;
static lv_obj_t *s_p_line3_ver = NULL;
static lv_obj_t *s_p_btn3 = NULL;/*btn2 center(20,300)*/

static lv_obj_t *s_p_line4_hor = NULL;
static lv_obj_t *s_p_line4_ver = NULL;
static lv_obj_t *s_p_btn4 = NULL;/*btn4 center(220,300)*/

static lv_obj_t *s_p_coord_label = NULL;/*coord label*/
static lv_obj_t *s_p_tip_label = NULL;/*tip label*/

static lv_point_t s_coord_val = {0};

static lv_point_t s_coord_real_val[CALIBRAION_POINT_COUNT] = {0};

static bool s_is_btn_calibrated[CALIBRAION_POINT_COUNT] = {false, false, false, false};

static int16_t s_calibrated_count = 0;

/* TODO: move to simulator stuck function */
#ifdef WIN32
typedef struct
{
    float   rate_x;
    float   rate_y;
    int16_t delta_x;
    int16_t delta_y;
}tp_cali_data_t;
#endif
/**********************
 *   STATIC FUNCTIONS
**********************/
static void tp_cali_update_coord(void)
{
    lv_point_t pot = {0};
    lv_indev_t *p_dev = lv_indev_get_act();

    if(NULL == p_dev) {
        LV_LOG_USER("dev is null");
        return;
    }
    lv_indev_get_point(p_dev,&pot);
    s_coord_val.x = pot.x;
    s_coord_val.y = pot.y;
}

static void set_cali_delta_val( void )
{
/* TODO: move to support indev */
#if CONFIG_TSC
    int32_t tmpX1=0, tmpX2=0;
    int32_t tmpY1=0, tmpY2=0;

    tp_cali_data_t cali_data;

    tmpX1 = (s_coord_real_val[0].x + s_coord_real_val[2].x)/(2);
    tmpX2 = (s_coord_real_val[1].x + s_coord_real_val[3].x)/(2);
    tmpY1 = (s_coord_real_val[0].y + s_coord_real_val[1].y)/(2);
    tmpY2 = (s_coord_real_val[2].y + s_coord_real_val[3].y)/(2);


    cali_data.rate_x = abs(tmpX1-tmpX2)/(240.0-20.0*2);
    cali_data.rate_y = abs(tmpY1-tmpY2)/(320.0-20.0*2);
    cali_data.delta_x = cali_data.rate_x*(((tmpX1>tmpX2?tmpX2:tmpX1)/cali_data.rate_x)-20);
    cali_data.delta_y = cali_data.rate_y*(((tmpY1>tmpY2?tmpY2:tmpY1)/cali_data.rate_y)-20);


#ifndef WIN32
    app_logi("rate_x rate_y [%d, %d]  delta x= delta y [%d, %d]\r\n",cali_data.rate_x,cali_data.rate_y,cali_data.delta_x,cali_data.delta_y);
#else
    LV_LOG_USER("rate_x rate_y [%d, %d]  delta x= delta y [%d, %d]\r\n",cali_data.rate_x,cali_data.rate_y,cali_data.delta_x,cali_data.delta_y);
#endif


    touchpad_set_cali_coordinate_data(&cali_data);
#endif
}



/* TODO: Del this */
extern lv_obj_t *win_em_tp;

static void msgbox_event_cb(lv_event_t * p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_btn = lv_event_get_target(p_event);
    lv_obj_t * p_msg_box = lv_obj_get_parent(p_btn);
    uint32_t key_val = lv_event_get_key(p_event);

    if(NULL == p_msg_box) {
        LV_LOG_ERROR("p_msg_box is null");
        return;
    }

    if((LV_EVENT_CLICKED == code && lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER) /*pointer:support click*/
        || (LV_EVENT_KEY == code && LV_KEY_ENTER == key_val)) {/*keypad:support enter quite UI*/
        const char * txt = lv_msgbox_get_active_btn_text(p_msg_box);
        if (0 == strncmp(txt, "OK", sizeof("OK"))) {
            set_cali_delta_val();
            if(NULL != p_msg_box) {
                lv_obj_del(p_msg_box);
            }
            s_calibrated_count = 0;
            //tp_cali_ctrls_del();
            //em_main_menu_appear();
            //em_calibration_btn_set_focus();
            alv_am_win_exit(win_em_tp, NULL, NULL);
        }
    }
}

static void msgbox_backspace_event_cb(lv_event_t * p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_btn = lv_event_get_target(p_event);
    lv_obj_t * p_msg_box = lv_obj_get_parent(p_btn);

    if (code == LV_EVENT_VALUE_CHANGED) {
        const char * txt = lv_msgbox_get_active_btn_text(p_msg_box);
        if (0 == strncmp(txt, "YES", sizeof("YES"))) {
            lv_msgbox_close(p_msg_box);
            s_calibrated_count = 0;
            //tp_cali_ctrls_del();
            //em_main_menu_appear();
            //em_calibration_btn_set_focus();
            alv_am_win_exit(win_em_tp, NULL, NULL);
        } else if (0 == strncmp(txt, "NO", sizeof("NO"))) {
            lv_msgbox_close(p_msg_box);
            lv_group_focus_obj(s_p_btn1);
        }
    }
}

static void tp_cali_btn_cb(lv_event_t *p_event)
{
    static lv_style_t style_line;
    char coord[200] = {0};

    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_btn = lv_event_get_user_data(p_event);
    uint32_t key_val = lv_event_get_key(p_event);

    static const char * btns[] ={"OK", ""};
    static const char *p_btns_backspace[] = {"YES", "NO", ""};

    bool is_all_btn_calibrated = true;
    uint8_t btn_index = 0;

    /* If press backspace key, pop up message box; else if press other key, do not response; . */
    if (LV_EVENT_KEY == code) {
        if (LV_KEY_BACKSPACE == key_val) {
            lv_obj_t *p_mbox_backspace = lv_msgbox_create(s_p_cont, NULL, "Your touchpad have not calibrated.\r\n Still exit?", p_btns_backspace, false);
            lv_group_focus_obj(lv_msgbox_get_btns(p_mbox_backspace));
            lv_obj_add_event_cb(p_mbox_backspace, msgbox_backspace_event_cb, LV_EVENT_ALL, NULL);
            lv_obj_center(p_mbox_backspace);
            return;
        } else {
            return;
        }
    }

    /* Only pointer input device could be responsed. */
    if ((LV_EVENT_PRESSED == code) && (lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER)) {
        tp_cali_update_coord();

        /*1.lines change to red*/
        lv_style_set_line_color(&style_line, lv_color_hex(0xff0000));
        if(s_p_btn1 == p_btn) {
            /* set btn1 calibrated flag */
            if (true != s_is_btn_calibrated[0]) {
                s_calibrated_count ++;
            }
            s_is_btn_calibrated[0] = true;

            lv_obj_add_style(s_p_line1_hor, &style_line, 0);
            lv_obj_add_style(s_p_line1_ver, &style_line, 0);

            /*calcu delta value*/
            s_coord_real_val[0].x = s_coord_val.x;
            s_coord_real_val[0].y = s_coord_val.y;
            LV_LOG_USER("delta.x=%d,y=%d",s_coord_real_val[0].x,s_coord_real_val[0].y);
        }
        else if (s_p_btn2 == p_btn) {
            /* set btn2 calibrated flag */
            if (true != s_is_btn_calibrated[1]) {
                s_calibrated_count ++;
            }
            s_is_btn_calibrated[1] = true;

            lv_obj_add_style(s_p_line2_hor, &style_line, 0);
            lv_obj_add_style(s_p_line2_ver, &style_line, 0);

            /*calcu delta value*/
            s_coord_real_val[1].x = s_coord_val.x;
            s_coord_real_val[1].y = s_coord_val.y;
            LV_LOG_USER("delta.x=%d,y=%d",s_coord_real_val[1].x,s_coord_real_val[1].y);
        }
        else if(s_p_btn3 == p_btn) {
            /* set btn3 calibrated flag */
            if (true != s_is_btn_calibrated[2]) {
                s_calibrated_count ++;
            }
            s_is_btn_calibrated[2] = true;

            lv_obj_add_style(s_p_line3_hor, &style_line, 0);
            lv_obj_add_style(s_p_line3_ver, &style_line, 0);

            /*calcu delta value*/
            s_coord_real_val[2].x = s_coord_val.x;
            s_coord_real_val[2].y = s_coord_val.y;
            LV_LOG_USER("delta.x=%d,y=%d",s_coord_real_val[2].x,s_coord_real_val[2].y);
        }
        else if(s_p_btn4 == p_btn) {
            /* set btn4 calibrated flag */
            if (true != s_is_btn_calibrated[3]) {
                s_calibrated_count ++;
            }
            s_is_btn_calibrated[3] = true;

            lv_obj_add_style(s_p_line4_hor, &style_line, 0);
            lv_obj_add_style(s_p_line4_ver, &style_line, 0);

            /*calcu delta value*/
            s_coord_real_val[3].x = s_coord_val.x;
            s_coord_real_val[3].y = s_coord_val.y;
            LV_LOG_USER("delta.x=%d,y=%d",s_coord_real_val[3].x,s_coord_real_val[3].y);
        }

        /*2.display this point from lvgl*/
        sprintf(coord,"You have calibrated %d points \r\n                   [%d,%d]",s_calibrated_count,s_coord_val.x,s_coord_val.y);
        lv_label_set_text(s_p_coord_label, coord);

        for (btn_index = 0; btn_index < CALIBRAION_POINT_COUNT; btn_index++) {
            if (false == s_is_btn_calibrated[btn_index]) {
                is_all_btn_calibrated = false;
                break;
            }
        }
        if (true == is_all_btn_calibrated) {
            lv_obj_t * mbox1 = lv_msgbox_create(s_p_cont, NULL, "Touchpad calibration complete.", btns, false);
            lv_obj_t * btn_ok = lv_msgbox_get_btns(mbox1);
            lv_obj_add_event_cb(btn_ok, msgbox_event_cb, LV_EVENT_ALL, NULL);
            lv_group_focus_obj(btn_ok);
            lv_obj_center(mbox1);
        }
    }
}

void em_tp_calibration_active(lv_obj_t *win, void *p_param)
{
#if CONFIG_TSC
    touchpad_clear_cali_coordinate_data();
#endif

    /*btn 1 line points*/
    static lv_point_t line1_points[] = { {CALI_POINT1_CENTER_X -(CALI_LINE_HOR_SIZE - 1)/2, CALI_POINT1_CENTER_Y}, {CALI_POINT1_CENTER_X +(CALI_LINE_HOR_SIZE - 1)/2, CALI_POINT1_CENTER_Y}};
    static lv_point_t line2_points[] = { {CALI_POINT1_CENTER_X,CALI_POINT1_CENTER_Y - (CALI_LINE_VER_SIZE -1)/2}, {CALI_POINT1_CENTER_X, CALI_POINT1_CENTER_Y + (CALI_LINE_VER_SIZE -1)/2}};

    static lv_point_t line3_points[] = { {CALI_POINT2_CENTER_X -(CALI_LINE_HOR_SIZE - 1)/2, CALI_POINT2_CENTER_Y}, {CALI_POINT2_CENTER_X +(CALI_LINE_HOR_SIZE - 1)/2, CALI_POINT2_CENTER_Y}};
    static lv_point_t line4_points[] = { {CALI_POINT2_CENTER_X,CALI_POINT2_CENTER_Y - (CALI_LINE_VER_SIZE -1)/2}, {CALI_POINT2_CENTER_X, CALI_POINT2_CENTER_Y + (CALI_LINE_VER_SIZE -1)/2}};

    static lv_point_t line5_points[] = { {CALI_POINT3_CENTER_X -(CALI_LINE_HOR_SIZE - 1)/2, CALI_POINT3_CENTER_Y}, {CALI_POINT3_CENTER_X +(CALI_LINE_HOR_SIZE - 1)/2, CALI_POINT3_CENTER_Y}};
    static lv_point_t line6_points[] = { {CALI_POINT3_CENTER_X,CALI_POINT3_CENTER_Y - (CALI_LINE_VER_SIZE -1)/2}, {CALI_POINT3_CENTER_X, CALI_POINT3_CENTER_Y + (CALI_LINE_VER_SIZE -1)/2}};

    static lv_point_t line7_points[] = { {CALI_POINT4_CENTER_X -(CALI_LINE_HOR_SIZE - 1)/2, CALI_POINT4_CENTER_Y}, {CALI_POINT4_CENTER_X +(CALI_LINE_HOR_SIZE - 1)/2, CALI_POINT4_CENTER_Y}};
    static lv_point_t line8_points[] = { {CALI_POINT4_CENTER_X,CALI_POINT4_CENTER_Y - (CALI_LINE_VER_SIZE -1)/2}, {CALI_POINT4_CENTER_X, CALI_POINT4_CENTER_Y + (CALI_LINE_VER_SIZE -1)/2}};

    /*Create style*/
    static lv_style_t style_line;
    static lv_style_t style_btn;

    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 1);
    lv_style_set_line_color(&style_line, lv_color_hex(0x000000));

    lv_style_init(&style_btn);
    lv_style_set_opa(&style_btn, LV_OPA_0);

    s_p_cont = win; /* All objects in this window are base on "win" */
    lv_obj_clear_flag(s_p_cont, LV_OBJ_FLAG_SCROLLABLE);


    /*draw btn1,btn1 center(20,20)*/
    s_p_btn1 = lv_btn_create(s_p_cont);
    lv_obj_set_pos(s_p_btn1, CALI_POINT1_CENTER_X - (CALI_BTN_SIZE-1)/2, CALI_POINT1_CENTER_Y - (CALI_BTN_SIZE-1)/2);
    lv_obj_set_size(s_p_btn1,CALI_BTN_SIZE,CALI_BTN_SIZE);
    lv_obj_add_style(s_p_btn1, &style_btn, 0); /*set button background is transparent*/

    s_p_line1_hor = lv_line_create(s_p_cont);
    lv_line_set_points(s_p_line1_hor, line1_points, 2);     /*Set the points*/
    lv_obj_add_style(s_p_line1_hor, &style_line, 0);
    s_p_line1_ver = lv_line_create(s_p_cont);
    lv_line_set_points(s_p_line1_ver, line2_points, 2);     /*Set the points*/
    lv_obj_add_style(s_p_line1_ver, &style_line, 0);

    lv_obj_add_event_cb(s_p_btn1, tp_cali_btn_cb, LV_EVENT_ALL, s_p_btn1);

    /*draw btn2 ,btn2 center(220,20)*/
    s_p_btn2 = lv_btn_create(s_p_cont);
    lv_obj_set_pos(s_p_btn2, CALI_POINT2_CENTER_X - (CALI_BTN_SIZE-1)/2, CALI_POINT2_CENTER_Y - (CALI_BTN_SIZE-1)/2);
    lv_obj_set_size(s_p_btn2,CALI_BTN_SIZE,CALI_BTN_SIZE);
    lv_obj_add_style(s_p_btn2, &style_btn, 0);
    s_p_line2_hor = lv_line_create(s_p_cont);
    lv_line_set_points(s_p_line2_hor, line3_points, 2);     /*Set the points*/
    lv_obj_add_style(s_p_line2_hor, &style_line, 0);
    s_p_line2_ver = lv_line_create(s_p_cont);
    lv_line_set_points(s_p_line2_ver, line4_points, 2);     /*Set the points*/
    lv_obj_add_style(s_p_line2_ver, &style_line, 0);
    lv_obj_add_event_cb(s_p_btn2, tp_cali_btn_cb, LV_EVENT_ALL, s_p_btn2);

    /*draw btn3 ,btn3 center(20,300)*/
    s_p_btn3 = lv_btn_create(s_p_cont);
    lv_obj_set_pos(s_p_btn3, CALI_POINT3_CENTER_X - (CALI_BTN_SIZE-1)/2, CALI_POINT3_CENTER_Y - (CALI_BTN_SIZE-1)/2);
    lv_obj_set_size(s_p_btn3,CALI_BTN_SIZE,CALI_BTN_SIZE);
    lv_obj_add_style(s_p_btn3, &style_btn, 0);
    s_p_line3_hor = lv_line_create(s_p_cont);
    lv_line_set_points(s_p_line3_hor, line5_points, 2);     /*Set the points*/
    lv_obj_add_style(s_p_line3_hor, &style_line, 0);
    s_p_line3_ver = lv_line_create(s_p_cont);
    lv_line_set_points(s_p_line3_ver, line6_points, 2);     /*Set the points*/
    lv_obj_add_style(s_p_line3_ver, &style_line, 0);
    lv_obj_add_event_cb(s_p_btn3, tp_cali_btn_cb, LV_EVENT_ALL, s_p_btn3);

    /*draw btn4 ,btn4 center(220,300)*/
    s_p_btn4 = lv_btn_create(s_p_cont);
    lv_obj_set_pos(s_p_btn4, CALI_POINT4_CENTER_X - (CALI_BTN_SIZE-1)/2, CALI_POINT4_CENTER_Y - (CALI_BTN_SIZE-1)/2);
    lv_obj_set_size(s_p_btn4,CALI_BTN_SIZE,CALI_BTN_SIZE);
    lv_obj_add_style(s_p_btn4, &style_btn, 0);
    s_p_line4_hor = lv_line_create(s_p_cont);
    lv_line_set_points(s_p_line4_hor, line7_points, 2);    /*Set the points*/
    lv_obj_add_style(s_p_line4_hor, &style_line, 0);
    s_p_line4_ver = lv_line_create(s_p_cont);
    lv_line_set_points(s_p_line4_ver, line8_points, 2);    /*Set the points*/
    lv_obj_add_style(s_p_line4_ver, &style_line, 0);
    lv_obj_add_event_cb(s_p_btn4, tp_cali_btn_cb, LV_EVENT_ALL, s_p_btn4);

    s_p_tip_label = lv_label_create(s_p_cont);

    lv_obj_align(s_p_tip_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(s_p_tip_label,"You need calibrate 4 points.\r\n          Please click + \r\n ");
    s_p_coord_label = lv_label_create(s_p_cont);
    lv_obj_align_to(s_p_coord_label,s_p_tip_label, LV_ALIGN_OUT_BOTTOM_LEFT, -2, 0);
    lv_label_set_text(s_p_coord_label,"You have calibrated 0 points \r\n                   [---]");

    lv_group_focus_obj(s_p_btn1);

    /* memset s_is_btn_calibrated array to 0 if enter win */
    memset(s_is_btn_calibrated, 0, sizeof(s_is_btn_calibrated));
}
