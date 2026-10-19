/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of engineering mode
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#ifndef WIN32
#include <fmd.h>
#include "display.h"
#include "aic_osa_ex.h"
#include "aic_errno.h"
#include "lv_graphics_hal.h"
#include "sysoff.h"
#endif

#include <display_cfg.h>

#include "em_version.h"
#include "tp_calibration.h"
#include "em_sketchpad.h"
#include "em_keypad.h"
#include "em_wifiscan.h"

#include "alv_app_mgr.h"

#include "win_emmenu/ui.h"
#include "win_em_deviceinfo/ui.h"
#include "win_em_device_version/ui.h"
#include "win_em_powerctrl/ui.h"
#include "win_em_teleinfo/ui.h"
#include "win_em_tele_siminfo/ui.h"
#include "win_em_tele_signalinfo/ui.h"
#include "win_em_tele_cellinfo/ui.h"
#include "win_em_tele_neighborinfo/ui.h"
#include "win_em_tele_regstatus/ui.h"
#include "win_em_tele_callsetting/ui.h"
#include "win_em_tele_smssetting/ui.h"
#include "win_em_logset/ui.h"

#ifdef CONFIG_CAMERA_SUPPORT
#include "camera_hal.h"
boolean g_is_em_camera_open = FALSE;
#endif

/*********************
 *      DEFINES
 *********************/

/* TODO: 后续用LCD 屏的宏替代*/
//#define SCREEN_WIDTH      (240)
//#define SCREEN_HEIGHT     (320)


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
//static void em_enter_win(void);

/**********************
 *  STATIC VARIABLES
 **********************/
#if 0
static lv_obj_t *p_em_list = NULL;
static lv_obj_t *s_em_menu_ptr = NULL;
static lv_obj_t *s_em_ver_btn = NULL;
static lv_obj_t *s_em_cali_btn = NULL;
static lv_obj_t *s_em_sketchpad_btn = NULL;
static lv_obj_t *s_em_poweroff_btn = NULL;
static lv_obj_t *s_em_powerreset_btn = NULL;
static lv_obj_t *s_em_keypad_btn_ptr = NULL;
static lv_obj_t *s_em_logo_btn = NULL;
static lv_obj_t *s_camerapreview_btn = NULL;


// TODO: del this
lv_obj_t *win_em_tp;
lv_obj_t *win_em_wifiscan;
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if 0
static void em_create_mask_btn(lv_obj_t *p_btn);

static void em_maskbtn_event_cb(lv_event_t *p_event);


/* TODO:this API move to em_camerapreview.c*/
#ifdef CONFIG_CAMERA_SUPPORT
extern u32 csi_rgb565_dst[];

void em_preview_refresh(u32 frame_addr, int width, int height, camera_data_fmt_e format)
{
    enum SCL_MODE scl_mode;

    if ((width == LCD_PANEL_WIDTH) && (height == LCD_PANEL_HEIGHT))
        scl_mode = NO_SCALER;
    else if ((width / LCD_PANEL_WIDTH == 2) && (height / LCD_PANEL_HEIGHT == 2))
        scl_mode = BINNING;
    else
        scl_mode = BILINEAR_SCALER;

    /* convert to rgb565 */
    yuv422_to_rgb565((unsigned char*)frame_addr, AIC_NULL,
                     width, height,
                     (unsigned short*)csi_rgb565_dst,
                     LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT,
                     MODE_YUYV, scl_mode,
                     0);

    display_refresh_sync((u32)csi_rgb565_dst);
}

static int em_camera_preview( void )
{
    int ret = -1;

    if (TRUE == g_is_em_camera_open) {
        app_logi("camera already open!\n");
        return ret;
    }

    ret = camera_open(CAMERA_FRONT);

    if (ret != 0) {
        app_logi("camera open failed!\n");
        return ret;
    }

    g_is_em_camera_open = TRUE;
    /* TODO:Use Camera service level API, no need care refresh logic */
    camera_register(em_preview_refresh);
    ret = camera_preview(CAMERA_FRONT, YUV_ONE_PLANE, YUV422, CAMERA_SENSOR_OUTPUT_WIDTH, CAMERA_SENSOR_OUTPUT_HEIGHT);
    if (ret != 0) {
        app_logi("camera preview failed!\n");
        return ret;
    }

    return 0;
}

static int em_camera_close( void )
{
    if (TRUE == g_is_em_camera_open) {
        camera_close(CAMERA_FRONT);
        g_is_em_camera_open = FALSE;
    }
    return AIC_SUCCESS;
}
#endif

/* TODO:this API move to em_logodisplay.c*/
static int em_show_logo( void )
{
#ifndef WIN32
#ifdef CONFIG_DISPLAY_SHOWLOGO
        extern int show_logo();
        show_logo();
#endif
#endif
    return 0;
}

static void em_item_camerapreview_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);
    lv_obj_t *p_main_menu = lv_event_get_user_data(p_event);

    if (LV_EVENT_KEY == code
        || (LV_EVENT_CLICKED == code && (LV_INDEV_TYPE_POINTER == lv_indev_get_type(lv_indev_get_act())))) {
        char c = *((char *)lv_event_get_param(p_event));
#ifndef WIN32
        app_logi(" c: %d", c);
#endif

        if (LV_KEY_ENTER == c
            || (LV_EVENT_CLICKED == code && (LV_INDEV_TYPE_POINTER == lv_indev_get_type(lv_indev_get_act())))) {

            /* TODO 进入相应的item测试窗*/
#ifdef CONFIG_CAMERA_SUPPORT
            em_camera_preview();
#endif
            em_create_mask_btn(p_obj);
        }
        else if (LV_KEY_BACKSPACE == c)
        {
#ifndef WIN32
            app_logi("c: %d", c);
#endif
#ifdef CONFIG_CAMERA_SUPPORT
            em_camera_close();
#endif
            lv_group_focus_obj(lv_menu_get_main_header_back_btn(p_main_menu));
            lv_event_send(lv_menu_get_main_header_back_btn(p_main_menu),LV_EVENT_CLICKED,NULL);
            lv_group_focus_obj(p_obj);

        }
    }
}

static void em_maskbtn_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);
    char c = *((char *)lv_event_get_param(p_event));
    lv_obj_t *p_target = lv_event_get_user_data(p_event);

    if (((LV_EVENT_KEY == code) && (LV_KEY_BACKSPACE == c))
        || ((LV_EVENT_RELEASED == code) && (lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER))) {
            if(s_em_logo_btn == p_target) {
#ifndef WIN32
                lv_set_refresh(true);
#endif
        }else if(s_camerapreview_btn == p_target) {
#ifndef WIN32
            app_logi("c: %d", c);
#endif
#ifdef CONFIG_CAMERA_SUPPORT
            em_camera_close();
#endif
        }
            lv_obj_del(p_obj);
            lv_group_focus_obj(p_target);
    }
}

static void em_create_mask_btn(lv_obj_t *p_btn)
{
    lv_style_t style_btn;
    lv_obj_t *s_em_mask_btn = NULL;

    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0xffffff));

    s_em_mask_btn = lv_btn_create(s_em_menu_ptr);

    lv_obj_set_size(s_em_mask_btn, LV_PCT(100),LV_PCT(100));
    lv_obj_set_style_bg_opa(s_em_mask_btn, LV_OPA_0, 0);

    lv_obj_add_event_cb(s_em_mask_btn,em_maskbtn_event_cb,LV_EVENT_KEY,p_btn);
    lv_obj_add_event_cb(s_em_mask_btn,em_maskbtn_event_cb,LV_EVENT_RELEASED,p_btn);
    lv_group_focus_obj(s_em_mask_btn);
}

static void em_item_logodisplay_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);
    lv_obj_t *p_main_menu = lv_event_get_user_data(p_event);

    if (LV_EVENT_KEY == code || LV_EVENT_CLICKED == code) {
        char c = *((char *)lv_event_get_param(p_event));
        LV_LOG_USER("c: %d", c);

        if (c == LV_KEY_ENTER || LV_EVENT_CLICKED == code) {

            /* TODO 进入相应的item测试窗*/
#ifndef WIN32
            lv_set_refresh(false);
            em_show_logo();
#endif
            em_create_mask_btn(p_obj);
        }
        else if (c == LV_KEY_BACKSPACE )
        {
#ifndef WIN32
            app_logi("c: %d", c);
            lv_set_refresh(true);
#endif
            lv_group_focus_obj(lv_menu_get_main_header_back_btn(p_main_menu));
            lv_event_send(lv_menu_get_main_header_back_btn(p_main_menu),LV_EVENT_CLICKED,NULL);
            lv_group_focus_obj(p_obj);
        }
    } else if (LV_EVENT_PRESSED == code ) {
        LV_LOG_USER("-------%s was pressed\n", lv_list_get_btn_text(p_em_list, p_obj));
    }
}

static void em_item_powertest_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);

    if ((LV_EVENT_KEY == code) || (LV_EVENT_CLICKED == code)) {
        char c = *((char *)lv_event_get_param(p_event));

            if ((c == LV_KEY_ENTER) || (LV_EVENT_CLICKED == code)) {
                if(s_em_poweroff_btn == p_obj) {
                        /*power off*/
#ifndef WIN32
                        sysoff_shutdown();
#endif
                    } else if(s_em_powerreset_btn == p_obj) {
                        /*power reset*/
#ifndef WIN32
                        sysoff_reset();
#endif
                    }
            }
        }
}

static void em_menu_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    if (LV_EVENT_CHILD_CREATED == code ) {
        /* TODO: Need to judge which API should be called. */
        em_version_set_focus();
    }
}

static void em_list_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    uint32_t c = lv_event_get_key(p_event);

    lv_obj_t *btn_obj = lv_event_get_target(p_event);
    lv_obj_t *list_obj = lv_obj_get_parent(btn_obj);
    uint32_t btn_idx = 0;
    uint32_t chd_cnt = 0;

    if (LV_EVENT_KEY == code) {
        btn_idx = lv_obj_get_index(btn_obj);
        chd_cnt = lv_obj_get_child_cnt(list_obj);
        if (LV_KEY_DOWN == c) {
            if(btn_idx == (chd_cnt-1)) { /*btn index is bottom*/
                return;
            }
            lv_group_focus_next(lv_group_get_default());
        } else if (LV_KEY_UP == c) {
            if(0 == btn_idx) { /*btn index is top*/
                return;
            }
            lv_group_focus_prev(lv_group_get_default());
        }
    }
}

static void em_cali_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    /*TODO:event click is OK ,but event press is not OK?*/
    if ( LV_EVENT_CLICKED == code ) {
       //lv_obj_add_flag(s_em_menu_ptr,LV_OBJ_FLAG_HIDDEN);
       //tp_calibration_enter_win();
       alv_am_win_enter(win_em_tp, NULL, NULL);
    }
}
static void em_item_wifiscan_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    /*TODO:event click is OK ,but event press is not OK?*/
    if ( LV_EVENT_CLICKED == code ) {
       alv_am_win_enter(win_em_wifiscan, NULL, NULL);
    }
}


static void em_sketchpad_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    if ( LV_EVENT_CLICKED == code ) {
       lv_obj_add_flag(s_em_menu_ptr, LV_OBJ_FLAG_HIDDEN);
       em_sketchpad_enter_win();
    }
}

static void em_item_keypad_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    if ( LV_EVENT_PRESSED == code ) {
       lv_obj_add_flag(s_em_menu_ptr, LV_OBJ_FLAG_HIDDEN);
       em_keypad_enter_win();
    }
}

#ifdef CONFIG_APP_PHOTO
static void em_item_photo_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    if ( LV_EVENT_CLICKED == code ) {
       lv_obj_add_flag(s_em_menu_ptr, LV_OBJ_FLAG_HIDDEN);
       alv_am_app_enter(NULL, "photo");
    }
}
#endif

#ifdef CONFIG_APP_FILEBROWSER
static void em_item_fb_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    if (LV_EVENT_PRESSED == code) {
        alv_am_app_enter(NULL, "filebrowser");
    }
}
#endif

#ifdef CONFIG_APP_MUSICPLAYER
static void em_item_mp_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    if (LV_EVENT_PRESSED == code) {
        alv_am_app_enter(NULL, "musicplayer");
    }
}
#endif

#ifdef CONFIG_APP_RECORDER
static void em_item_rec_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    if (LV_EVENT_PRESSED == code) {
        alv_am_app_enter(NULL, "recorder");
    }
}
#endif

#ifdef CONFIG_APP_FOTA
static void em_item_app_fota_event_cb(lv_event_t *p_event) {
    lv_event_code_t code = lv_event_get_code(p_event);

    if (LV_EVENT_CLICKED == code) {
       alv_am_app_enter(NULL, "app_fota");
    }
}
#endif

static void em_menu_active(lv_obj_t *win, void *p_param)
{
    lv_obj_t *p_main_page = NULL;
    lv_obj_t *p_sub_version_page = NULL;
    lv_obj_t *p_menu_back_btn = NULL;
    static lv_obj_t *p_em_wifiscan_btn = NULL;

    s_em_menu_ptr = lv_menu_create(win);
    lv_obj_add_event_cb(s_em_menu_ptr, em_menu_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_size(s_em_menu_ptr, SCREEN_WIDTH, SCREEN_HEIGHT);  /*menu size*/
    lv_obj_center(s_em_menu_ptr);

    p_menu_back_btn = lv_menu_get_main_header_back_btn(s_em_menu_ptr);
    lv_obj_set_width(p_menu_back_btn, LV_PCT(30));           /*back btn size*/

    /*Create a main page*/
    p_main_page = lv_menu_page_create(s_em_menu_ptr,"Engineering Mode");

    /*Create a list*/
    p_em_list = lv_list_create(p_main_page);
    lv_obj_add_event_cb(p_em_list, em_list_event_cb, LV_EVENT_KEY, NULL);

    /*Add buttons to the list*/
    /*1.Version info*/
    s_em_ver_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_EYE_OPEN, "Version Info");
    p_sub_version_page = em_version_enter_win(s_em_menu_ptr);
    lv_menu_set_load_page_event(s_em_menu_ptr,s_em_ver_btn,p_sub_version_page);
    /* LV_KEY_UP & LV_KEY_DOWN need to be received by parent list, so add flag LV_OBJ_FLAG_EVENT_BUBBLE. */
    lv_obj_add_flag(s_em_ver_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

    /*2.Camera Preview*/
    s_camerapreview_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_VIDEO, "Camera Preview");
    lv_obj_add_event_cb(s_camerapreview_btn, em_item_camerapreview_event_cb,LV_EVENT_ALL, s_em_menu_ptr);
    lv_obj_add_flag(s_camerapreview_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

    /*3.Logo Display*/
    s_em_logo_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_IMAGE, "Logo Display");
    lv_obj_add_event_cb(s_em_logo_btn, em_item_logodisplay_event_cb,LV_EVENT_ALL, s_em_menu_ptr);
    lv_obj_add_flag(s_em_logo_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

    /*4.Keypad Test*/
    s_em_keypad_btn_ptr = lv_list_add_btn(p_em_list, LV_SYMBOL_KEYBOARD, "Keypad Test");
    lv_obj_add_event_cb(s_em_keypad_btn_ptr, em_item_keypad_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_flag(s_em_keypad_btn_ptr, LV_OBJ_FLAG_EVENT_BUBBLE);

    /*5.TP Calibration Test*/
    s_em_cali_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_PLUS, "Calibration Test");
    lv_obj_add_event_cb(s_em_cali_btn, em_cali_event_cb,LV_EVENT_ALL, s_em_menu_ptr);
    lv_obj_add_flag(s_em_cali_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

    /*6.Sketchpad Test*/
    s_em_sketchpad_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_EDIT, "Sketchpad Test");
    lv_obj_add_event_cb(s_em_sketchpad_btn, em_sketchpad_event_cb, LV_EVENT_ALL, s_em_menu_ptr);
    lv_obj_add_flag(s_em_sketchpad_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

    /*8.Power Off*/
    s_em_poweroff_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_POWER, "Power Off");
    lv_obj_add_event_cb(s_em_poweroff_btn, em_item_powertest_event_cb,LV_EVENT_ALL, s_em_menu_ptr);
    lv_obj_add_flag(s_em_poweroff_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

    /*9.Power Reset*/
    s_em_powerreset_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_POWER, "Power Reset");
    lv_obj_add_event_cb(s_em_powerreset_btn, em_item_powertest_event_cb,LV_EVENT_ALL, s_em_menu_ptr);
    lv_obj_add_flag(s_em_powerreset_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

    /*10.WiFi Scan*/
    p_em_wifiscan_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_WIFI, "WiFi Scan");
    lv_obj_add_event_cb(p_em_wifiscan_btn, em_item_wifiscan_event_cb,LV_EVENT_ALL, NULL);
    lv_obj_add_flag(p_em_wifiscan_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

#ifdef CONFIG_APP_PHOTO
    /*11.Photo*/
    lv_obj_t *em_photo_btn = lv_list_add_btn(p_em_list, LV_SYMBOL_IMAGE, "Photo");
    lv_obj_add_event_cb(em_photo_btn, em_item_photo_event_cb,LV_EVENT_ALL, NULL);
    lv_obj_add_flag(em_photo_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
#endif

#ifdef CONFIG_APP_FILEBROWSER
    lv_obj_t *em_fb_btn =
        lv_list_add_btn(p_em_list, LV_SYMBOL_SD_CARD, "File Browser");
    if (em_fb_btn) {
        lv_obj_add_event_cb(em_fb_btn, em_item_fb_event_cb, LV_EVENT_ALL, NULL);
        lv_obj_clear_flag(em_fb_btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_add_flag(em_fb_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    }
#endif

#ifdef CONFIG_APP_MUSICPLAYER
    lv_obj_t *em_mp_btn =
        lv_list_add_btn(p_em_list, LV_SYMBOL_AUDIO, "Music player");
    if (em_mp_btn) {
        lv_obj_add_event_cb(em_mp_btn, em_item_mp_event_cb, LV_EVENT_ALL, NULL);
        lv_obj_clear_flag(em_mp_btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_add_flag(em_mp_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    }
#endif

#ifdef CONFIG_APP_RECORDER
    lv_obj_t *em_rec_btn =
        lv_list_add_btn(p_em_list, LV_SYMBOL_PLUS, "Recorder");
    if (em_rec_btn) {
        lv_obj_add_event_cb(em_rec_btn, em_item_rec_event_cb, LV_EVENT_ALL, NULL);
        lv_obj_clear_flag(em_rec_btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_add_flag(em_rec_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    }
#endif

#ifdef CONFIG_APP_FOTA
    lv_obj_t *em_fota_btn =
        lv_list_add_btn(p_em_list, LV_SYMBOL_DOWNLOAD, "FOTA");
    if (em_fota_btn) {
       lv_obj_add_event_cb(em_fota_btn, em_item_app_fota_event_cb, LV_EVENT_ALL,
                           NULL);
       lv_obj_clear_flag(em_fota_btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
       lv_obj_add_flag(em_fota_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    }
#endif

    lv_menu_set_page(s_em_menu_ptr,p_main_page);
    lv_group_focus_obj(s_em_ver_btn);

    /* Todo: Moify the resource of List Icon */
    /* Todo: Add action in event_handler, to call APIs */

}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * em_version_btn_set_focus() - set version btn focus.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void em_version_btn_set_focus(void)
{
    lv_group_focus_obj(s_em_ver_btn);
}

/**
* em_keypad_btn_set_focus - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to focus keypad btn object.
*              TODO: This is a work around solution.
*/
void em_keypad_btn_set_focus(void)
{
    lv_group_focus_obj(s_em_keypad_btn_ptr);
}

/**
 * em_calibration_btn_set_focus() - the calibration btn focus.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void em_calibration_btn_set_focus(void)
{
    lv_group_focus_obj(s_em_cali_btn);
}

/**
 * em_init() - the init of engineering mode.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void em_main_menu_appear(void)
{
    lv_obj_clear_flag(s_em_menu_ptr,LV_OBJ_FLAG_HIDDEN);
}
#endif
/**
 * em_init() - the init of engineering mode.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void em_init(void)
{
    app_logi("The engineering model has been initialled in [%s]\n", __func__);

    /* Create APP for engineering mode */
    lv_obj_t *app_em = alv_am_app_create(lv_scr_act(), "em");

    /* Main Window is a menu win which show Items for EM */
    lv_obj_t *win_em_menu = alv_am_win_create(app_em, "em_mainmenu");
    alv_am_win_set_active(win_em_menu, emmenu_win_active);
    alv_am_win_set_deactive(win_em_menu, emmenu_win_deactive);
    alv_am_win_set_focus(win_em_menu, emmenu_win_focus);
    alv_am_win_set_defocus(win_em_menu, emmenu_win_defocus);

    /* device info is device related settings */
    lv_obj_t *win_em_deviceinfo = alv_am_win_create(app_em, "em_deviceinfo");
    alv_am_win_set_active(win_em_deviceinfo, emdeviceinfo_win_active);
    alv_am_win_set_deactive(win_em_deviceinfo, emdeviceinfo_win_deactive);
    alv_am_win_set_focus(win_em_deviceinfo, emdeviceinfo_win_focus);
    alv_am_win_set_defocus(win_em_deviceinfo, emdeviceinfo_win_defocus);

    /* device version info is device version infomation */
    lv_obj_t *win_em_versioninfo = alv_am_win_create(app_em, "em_device_version");
    alv_am_win_set_active(win_em_versioninfo, em_device_version_win_active);
    alv_am_win_set_deactive(win_em_versioninfo, em_device_version_win_deactive);
    alv_am_win_set_focus(win_em_versioninfo, em_device_version_win_focus);

    /* power control is power related settings */
    lv_obj_t *win_em_pwrctrl = alv_am_win_create(app_em, "em_powerctrl");
    alv_am_win_set_active(win_em_pwrctrl, em_pwrctrl_win_active);
    alv_am_win_set_deactive(win_em_pwrctrl, em_pwrctrl_win_deactive);
    alv_am_win_set_focus(win_em_pwrctrl, em_pwrctrl_win_focus);

    /* tele info is tele related settings */
    lv_obj_t *win_em_teleinfo = alv_am_win_create(app_em, "em_teleinfo");
    alv_am_win_set_active(win_em_teleinfo, em_teleinfo_win_active);
    alv_am_win_set_deactive(win_em_teleinfo, em_teleinfo_win_deactive);
    alv_am_win_set_focus(win_em_teleinfo, em_teleinfo_win_focus);
    alv_am_win_set_defocus(win_em_teleinfo, em_teleinfo_win_defocus);

    /* tele sim info is sim related information */
    lv_obj_t *win_em_tele_siminfo = alv_am_win_create(app_em, "em_tele_siminfo");
    alv_am_win_set_active(win_em_tele_siminfo, em_tele_siminfo_win_active);
    alv_am_win_set_deactive(win_em_tele_siminfo, em_tele_siminfo_win_deactive);
    alv_am_win_set_focus(win_em_tele_siminfo, em_tele_siminfo_win_focus);

    /* tele signal info is sim signal information */
    lv_obj_t *win_em_tele_signalinfo = alv_am_win_create(app_em, "em_tele_signalinfo");
    alv_am_win_set_active(win_em_tele_signalinfo, em_tele_signalinfo_win_active);
    alv_am_win_set_deactive(win_em_tele_signalinfo, em_tele_signalinfo_win_deactive);
    alv_am_win_set_focus(win_em_tele_signalinfo, em_tele_signalinfo_win_focus);

    /* tele cell info is cell related information */
    lv_obj_t *win_em_tele_cellinfo = alv_am_win_create(app_em, "em_tele_cellinfo");
    alv_am_win_set_active(win_em_tele_cellinfo, em_tele_cellinfo_win_active);
    alv_am_win_set_deactive(win_em_tele_cellinfo, em_tele_cellinfo_win_deactive);
    alv_am_win_set_focus(win_em_tele_cellinfo, em_tele_cellinfo_win_focus);

    /* tele neighbor info is cell related information */
    lv_obj_t *win_em_tele_neighborinfo = alv_am_win_create(app_em, "em_tele_ncellinfo");
    alv_am_win_set_active(win_em_tele_neighborinfo, em_tele_neighborinfo_win_active);
    alv_am_win_set_deactive(win_em_tele_neighborinfo, em_tele_neighborinfo_win_deactive);
    alv_am_win_set_focus(win_em_tele_neighborinfo, em_tele_neighborinfo_win_focus);
    /* tele reg status is radio register status information */
    lv_obj_t *win_em_tele_regstatus = alv_am_win_create(app_em, "em_tele_regstatus");
    alv_am_win_set_active(win_em_tele_regstatus, em_tele_regstatus_win_active);
    alv_am_win_set_deactive(win_em_tele_regstatus, em_tele_regstatus_win_deactive);
    alv_am_win_set_focus(win_em_tele_regstatus, em_tele_regstatus_win_focus);

    /* tele call setting is call related setting */
    lv_obj_t *win_em_tele_callsetting = alv_am_win_create(app_em, "em_tele_callsetting");
    alv_am_win_set_active(win_em_tele_callsetting, em_tele_callsetting_win_active);
    alv_am_win_set_deactive(win_em_tele_callsetting, em_tele_callsetting_win_deactive);
    alv_am_win_set_focus(win_em_tele_callsetting, em_tele_callsetting_win_focus);

    /* tele sms setting is sms related setting */
    lv_obj_t *win_em_tele_smssetting = alv_am_win_create(app_em, "em_tele_smssetting");
    alv_am_win_set_active(win_em_tele_smssetting, em_tele_smssetting_win_active);
    alv_am_win_set_deactive(win_em_tele_smssetting, em_tele_smssetting_win_deactive);
    alv_am_win_set_focus(win_em_tele_smssetting, em_tele_smssetting_win_focus);

    /* log set is set log level */
    lv_obj_t *win_em_logset = alv_am_win_create(app_em, "em_logset");
    alv_am_win_set_active(win_em_logset, em_logset_win_active);
    alv_am_win_set_deactive(win_em_logset, em_logset_win_deactive);
    alv_am_win_set_focus(win_em_logset, em_logset_win_focus);

    alv_am_app_set_first_win(app_em, win_em_menu);

#if 0
    /*----- Sub Windows ------*/
    /*----- 5. TP Calibration  -----*/
    win_em_tp = alv_am_win_create(app_em, "em_tp");
    alv_am_win_set_active(win_em_tp, em_tp_calibration_active);

    /*----- Sub Windows ------*/
    /*----- 10. WiFi Scan  -----*/
    win_em_wifiscan = alv_am_win_create(app_em,"Wifi Scan");
    alv_am_win_set_active(win_em_wifiscan, em_wifiscan_active);

    //alv_am_app_enter(app_em, NULL);
#endif
}


