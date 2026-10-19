/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is enter of engineering mode version
*/

/*********************
 *      INCLUDES
*********************/
#include "em.h"
#include "lvgl.h"

#ifndef WIN32 
#include <aic_version.h>
#endif

/*********************
 *      DEFINES
*********************/
#define VERSION_MAX_LEN  (120)

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t *s_em_menu_ptr = NULL;
static lv_obj_t *s_version_table_ptr = NULL;

/**********************
 *   STATIC FUNCTIONS
**********************/
static void table_event_cb(lv_event_t *p_event)
{
    uint32_t key = 0;
    lv_event_code_t code = lv_event_get_code(p_event);

    key = lv_event_get_key(p_event);
    if (0 == key) {
        return;
    }

    if (LV_EVENT_KEY == code) {
        if (LV_KEY_BACKSPACE == key) {
            lv_group_focus_obj(lv_menu_get_main_header_back_btn(s_em_menu_ptr));
            lv_event_send(lv_menu_get_main_header_back_btn(s_em_menu_ptr), LV_EVENT_CLICKED, NULL);
            em_version_btn_set_focus();
        }
    }
}

/**********************
 *   GLOBAL FUNCTIONS
**********************/
void em_version_set_focus(void)
{
    lv_group_focus_obj(s_version_table_ptr);
}

lv_obj_t *em_version_enter_win( lv_obj_t *p_menu )
{
    lv_obj_t              *p_sub_version_page = NULL;
    lv_obj_t              *p_sub_version_cont = NULL;

    char char_ver[VERSION_MAX_LEN] = {0};
    size_t                 ver_cnt = VERSION_MAX_LEN;

    s_em_menu_ptr = p_menu;

    /*Create a sub(version info) page*/
    p_sub_version_page = lv_menu_page_create(p_menu,"Version Info");
    p_sub_version_cont = lv_menu_cont_create(p_sub_version_page);

    s_version_table_ptr = lv_table_create(p_sub_version_cont);
    lv_table_set_col_width(s_version_table_ptr, 0, 215);
#ifndef WIN32
    aic_get_version(char_ver,ver_cnt);
#endif
    lv_table_set_cell_value(s_version_table_ptr, 0, 0, char_ver);
    lv_obj_add_event_cb(s_version_table_ptr, table_event_cb, LV_EVENT_ALL, NULL);

    return p_sub_version_page;

}
