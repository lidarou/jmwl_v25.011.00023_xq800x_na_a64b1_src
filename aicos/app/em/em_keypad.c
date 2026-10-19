/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  @file em_keypad.c
 *
 *    This file is enter of keypad UI about engineering mode
 */

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>
#include <aic_osa_ex.h>
#include "em.h"
#include "lv_port_indev_keypad.h"

/*********************
 *      DEFINES
 *********************/
#define KEYPAD_BACK_KEY_BTN_IDX       (2)
#ifdef WIN32
#define KEYPAD_GREEN_KEY_BTN_IDX      (8)
#define KEYPAD_RED_KEY_BTN_IDX        (10)
#endif

#define KEYPAD_BTN_MATRIX_WIDTH_EXTRA (20)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void btnmatrix_event_cb(lv_event_t *p_event);

/**********************
 *  STATIC VARIABLES
 **********************/
static const char *s_btnmatrix_map_ptr[] = {"ESC",   "Up",    "Back",  "Del",  "\n",
                                            "Left",  "Enter", "Right", "Home", "\n",
                                            "Green", "Down",  "Red",   "End",  ""};

static const uint32_t s_btnmatrix_lv_key[] = {LV_KEY_ESC,   LV_KEY_UP,    LV_KEY_BACKSPACE, LV_KEY_DEL,
                                              LV_KEY_LEFT,  LV_KEY_ENTER, LV_KEY_RIGHT,     LV_KEY_HOME,
                                              LV_KEY_GREEN, LV_KEY_DOWN,  LV_KEY_RED,       LV_KEY_END};

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void btnmatrix_event_cb(lv_event_t *p_event)
{
    uint32_t key = 0;
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);
    uint8_t i = 0;
    bool is_all_btns_pressed = true;

    if (LV_EVENT_KEY == code) {
        key = lv_event_get_key(p_event);
#ifndef WIN32
        app_logi("[%s] code = %d, key = %d.\n", __func__, code, key);
#endif

        /* if back button is selected, press back or enter both return to up level UI. */
        if (KEYPAD_BACK_KEY_BTN_IDX == lv_btnmatrix_get_selected_btn(p_obj)) {
            if ((LV_KEY_ENTER == key) || (LV_KEY_BACKSPACE == key) || (LV_KEY_DEL == key)) {
                lv_obj_del(p_obj);
                return;
            }
        }

        /* if all btns have been pressed, press back button will return to up level UI */
        if (LV_KEY_BACKSPACE == key) {
            for (i = 0; i < (sizeof(s_btnmatrix_lv_key) / sizeof(uint32_t)); i++) {
                if (false == lv_btnmatrix_has_btn_ctrl(p_obj, i, LV_BTNMATRIX_CTRL_CHECKED)) {
                    is_all_btns_pressed = false;
                }
            }
            if (true == is_all_btns_pressed) {
                lv_obj_del(p_obj);
                return;
            }
        }

        for (i = 0; i < (sizeof(s_btnmatrix_lv_key) / sizeof(uint32_t)); i++) {
            if (key == s_btnmatrix_lv_key[i]) {
                lv_btnmatrix_set_btn_ctrl(p_obj, i, LV_BTNMATRIX_CTRL_CHECKED);
                lv_btnmatrix_set_selected_btn(p_obj, i);
                break;
            }
        }
    }
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
* em_keypad_enter_win - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to enter em keypad test UI.
*/
void em_keypad_enter_win(void)
{
    lv_obj_t *p_btnmatrix = lv_btnmatrix_create(lv_scr_act());

    /* set button matrix display area */
    lv_obj_set_size(p_btnmatrix, lv_disp_get_hor_res(NULL) + KEYPAD_BTN_MATRIX_WIDTH_EXTRA, lv_disp_get_ver_res(NULL) / 2);

    /* set button matrix map */
    lv_btnmatrix_set_map(p_btnmatrix, s_btnmatrix_map_ptr);
    lv_obj_align(p_btnmatrix, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(p_btnmatrix, btnmatrix_event_cb, LV_EVENT_KEY, NULL);

#ifdef WIN32
    /* TODO: research more key for lv */
    lv_btnmatrix_set_btn_ctrl(p_btnmatrix, KEYPAD_GREEN_KEY_BTN_IDX, LV_BTNMATRIX_CTRL_DISABLED);
    lv_btnmatrix_set_btn_ctrl(p_btnmatrix, KEYPAD_RED_KEY_BTN_IDX, LV_BTNMATRIX_CTRL_DISABLED);
#endif

    lv_group_focus_obj(p_btnmatrix);
}
