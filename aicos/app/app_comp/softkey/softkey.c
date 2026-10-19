/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "softkey.h"

#ifdef CONFIG_RESOLUTION_128x128

#else
#include "comp_softkey/ui.h"
#endif

/**********************
 *   STATIC VARIABLES
 **********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void comp_softkey_show( softkey_t *left_softkey, softkey_t *center_softkey, softkey_t *right_softkey)
{
    lv_obj_t *p_cur_scr = lv_scr_act();

    /* Del old softkey and Initnal new softkey if ui_PnlSoftkey is deleted*/
    if(lv_obj_is_valid(ui_PnlSoftKey) == false || (NULL == ui_PnlSoftKey)) {
        ui_softkey_init();
    }

    if (lv_obj_get_screen(ui_PnlSoftKey) != p_cur_scr) {
        lv_obj_set_parent(ui_PnlSoftKey, p_cur_scr);
        lv_obj_clear_flag(ui_PnlSoftKey, LV_OBJ_FLAG_HIDDEN);
    }

    if(NULL == left_softkey) {
        lv_obj_add_flag(ui_BtnLeft, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ui_BtnLeft, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_LblLeft, left_softkey->p_txt);
    }

    if(NULL == center_softkey) {
        lv_obj_add_flag(ui_BtnCenter, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ui_BtnCenter, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_LblCenter, center_softkey->p_txt);
    }

    if(NULL == right_softkey) {
        lv_obj_add_flag(ui_BtnRight, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ui_BtnRight, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_LblRight, right_softkey->p_txt);
    }
}
/*TODO: add_event_cb and image support*/

void comp_softkey_init(void)
{
    ui_softkey_init();
}
